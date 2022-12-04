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

export type GraphIncomingEntry = {
  fromStr: StopID;
  tripStr: TripID;
};

export type TravelTime = {
  totalNrPeople: number;
  optimalNrPeople: number;
  medianTravelTime: number;
  medianTravelTimeFormatted: string;
  geojson: FeatureCollection;
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

  private async getJson<T>(url: string): Promise<T> {
    return new Promise((resolve, reject) => {
      this.client({ url })
        .then(response => {
          const error = API.checkError(response);

          if (error !== null) {
            reject(error);
          }
          resolve(response.data);
        })
        .catch(error => {
          reject(error);
        });
    });
  }

  async stops(): Promise<Stops> {
    return this.getJson<Stops>('/stops');
  }

  async relativeTravelTime(stopId: StopID): Promise<FeatureCollection> {
    return this.getJson<FeatureCollection>(`/travelTimeLayer/${stopId}`);
  }

  async graphFrom(stopId: StopID): Promise<Graph> {
    return this.getJson<Graph>(`/graphFrom/${stopId}`);
  }

  async travelDistance(stopId: StopID): Promise<TravelDistance> {
    return this.getJson<TravelDistance>(`/travelDistance/${stopId}`);
  }

  async travelTime(stopId: StopID): Promise<TravelTime> {
    return this.getJson<TravelTime>(`/travelTime/${stopId}`);
  }
}

export default API;
