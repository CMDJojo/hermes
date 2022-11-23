const API_ENDPOINT = import.meta.env.VITE_API_ENDPOINT;

export async function getStops() {
  return fetch(`${API_ENDPOINT}/stops`).then((res: Response) => res.json());
}

export default {};
