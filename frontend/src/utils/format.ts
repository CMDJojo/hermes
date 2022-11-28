export default function formatDistance(distance: number): string {
  if (distance >= 1000) {
    return `${Math.round(distance / 1000)} km`;
  }

  return `${Math.round(distance / 10) * 10} m`;
}
