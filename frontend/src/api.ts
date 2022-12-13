import axios, { AxiosInstance, AxiosResponse } from 'axios';
import { FeatureCollection } from 'geojson';

export type Feature = {
  geometry: {
    coordinates: [number, number];
  };
  id: number;
  properties: {
    name: string;
  };
};

export type Stops = {
  features: Feature[];
};

export type StopID = string;
export type TripID = string;

export type APIOptions = {
  timetableId?: number;
  date?: Date;
};

export type GraphIncomingEntry = {
  fromStr: StopID;
  tripStr: TripID;
};

export type Timetable = {
  name: string;
  id: number;
  startDate: { year: number; month: number; day: number };
  endDate: { year: number; month: number; day: number };
};

export type TravelTime = {
  totalNrPeople: number;
  peopleCanGoByBus: number;
  optimalNrPeople: number;
  interestingStopID: string;
  travelTimeStats: { name: string; people: number }[];
  peopleTravelFrom: {
    stopID: StopID;
    stopName: string;
    numberOfPersons: number;
  }[];
  medianTravelTime: number;
  medianTravelTimeFormatted: string;
  lines: FeatureCollection;
  walks: FeatureCollection;
  numberOfTransfers: number;
  transfers: {
    stopID: StopID;
    stopName: string;
    percentage: number;
  }[];
};

export type GraphEntry = {
  time: number;
  incoming: GraphIncomingEntry[];
};

export type Graph = Record<StopID, GraphEntry>;

export type TravelDistance = {
  nrPeople: number;
  peopleRange: number;
  distanceStats: { name: string; distance: number }[];
  medianDistance: number;
};

class APIError extends Error {
  private response: AxiosResponse | null;

  constructor(message: string, response: AxiosResponse | null) {
    super(message);

    this.response = response;
  }
}

class ServerError extends APIError {
  constructor(response: AxiosResponse) {
    super('The server is puckoprogrammerad', response);
  }
}

class NotFoundError extends APIError {
  constructor(response: AxiosResponse) {
    super('The requested URL could not be found', response);
  }
}

class AuthenticationError extends APIError {
  constructor(response: AxiosResponse) {
    super('The request could not be authenticated', response);
  }
}

class ClientError extends APIError {
  constructor(response: AxiosResponse) {
    super('The request failed', response);
  }
}

class NoDataError extends APIError {
  constructor(response: AxiosResponse) {
    super('The API call yielded no response', response);
  }
}

class API {
  private readonly client: AxiosInstance;

  constructor() {
    this.client = axios.create({
      method: 'GET',
      baseURL: import.meta.env.VITE_API_ENDPOINT,
      responseType: 'json',
      timeout: 3000,
    });
  }

  private static checkError(response: AxiosResponse): APIError | null {
    if (response.status >= 500) {
      return new ServerError(response);
    }

    if (response.status === 404) {
      return new NotFoundError(response);
    }

    if (response.status === 403) {
      return new AuthenticationError(response);
    }

    if (response.status < 200 || response.status >= 300) {
      return new ClientError(response);
    }

    if (!response.data) {
      return new NoDataError(response);
    }

    return null;
  }

  private async getJson<T>(url: string, parse?: (value: any) => T): Promise<T> {
    return new Promise((resolve, reject) => {
      this.client({ url })
        .then(response => {
          const error = API.checkError(response);

          if (error !== null) {
            reject(error);
          }
          if (parse !== undefined) {
            resolve(parse(response.data));
          } else {
            resolve(response.data);
          }
        })
        .catch(error => {
          reject(error);
        });
    });
  }

  async stops(): Promise<Stops> {
    return this.getJson<Stops>('/stops');
  }

  optionsToQuery(options?: APIOptions): string {
    if (options === undefined) return '';

    const query = new URLSearchParams();
    /*
    The backend supports the following params
    date
    time
    searchTime
    minTransferTime
    timetable
    */

    // The "date" param is stored in the format YYYYMMDD
    if (options.date !== undefined && options.date !== null) {
      const year = options.date.getFullYear().toString();
      const month = (options.date.getMonth() + 1).toString().padStart(2, '0');
      const day = options.date.getDate().toString().padStart(2, '0');
      query.set('date', `${year}${month}${day}`);

      // The "time" is the number of seconds since midnight
      const time =
        options.date.getHours() * 3600 + options.date.getMinutes() * 60;
      query.set('time', time.toString());
    }

    if (options.timetableId !== undefined && options.timetableId !== null) {
      query.set('timetable', options.timetableId.toString());
    }

    return query.toString();
  }

  async relativeTravelTime(
    stopId: StopID,
    options?: APIOptions
  ): Promise<FeatureCollection> {
    return this.getJson<FeatureCollection>(
      `/travelTimeLayer/${stopId}?${this.optionsToQuery(options)}`
    );
  }

  async graphFrom(stopId: StopID, options?: APIOptions): Promise<Graph> {
    return this.getJson<Graph>(
      `/graphFrom/${stopId}?${this.optionsToQuery(options)}`
    );
  }

  async travelDistance(
    stopId: StopID,
    options?: APIOptions
  ): Promise<TravelDistance> {
    return this.getJson<TravelDistance>(
      `/travelDistance/${stopId}?${this.optionsToQuery(options)}`
    );
  }

  async travelTime(stopId: StopID, options?: APIOptions): Promise<TravelTime> {
    return this.getJson<TravelTime>(
      `/travelTime/${stopId}?${this.optionsToQuery(options)}`
    );
  }

  async timetables(options?: APIOptions): Promise<Timetable[]> {
    return this.getJson<Timetable[]>(
      `/timetables?${this.optionsToQuery(options)}`,
      data => {
        return data.timetables.map((timetable: any) => ({
          name: timetable.name,
          id: timetable.id,
          // startDate stored in a number on the form of YYYYMMDD
          startDate: {
            year: parseInt(timetable.startDate.toString().slice(0, 4), 10),
            month: parseInt(timetable.startDate.toString().slice(4, 6), 10),
            day: parseInt(timetable.startDate.toString().slice(6, 8), 10),
          },
          // endDate stored in a number on the form of YYYYMMDD
          endDate: {
            year: parseInt(timetable.endDate.toString().slice(0, 4), 10),
            month: parseInt(timetable.endDate.toString().slice(4, 6), 10),
            day: parseInt(timetable.endDate.toString().slice(6, 8), 10),
          },
        }));
      }
    );
  }
}

export default API;
