import axios, { AxiosInstance, AxiosResponse } from 'axios';

type Stops = null;

class APIError extends Error {
  private response: AxiosResponse | null;

  constructor(message: string, response: AxiosResponse | null) {
    super(message);

    this.response = response;
  }
}

class NoDataError extends APIError {
  constructor(response: AxiosResponse) {
    super('The API call yielded no response', response);
  }
}

class ServerError extends APIError {
  constructor() {
    super('The server is puckoprogrammerad', null);
  }
}

class API {
  private client: AxiosInstance;

  constructor() {
    this.client = axios.create({
      method: 'GET',
      baseURL: import.meta.env.VITE_API_ENDPOINT,
      responseType: 'json',
      timeout: 3000,
      validateStatus: status => status >= 200 && status < 300,
    });
  }

  async stops(): Promise<Stops> {
    return new Promise((resolve, reject) => {
      this.client({
        url: '/stops',
      }).then(response => {
        if (!response.data) {
          reject(new NoDataError(response));
        }
        resolve(response.data);
      });
    });
  }
}

export default API;
