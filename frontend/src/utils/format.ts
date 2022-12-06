export default function formatDistance(distance: number): string {
  if (distance >= 10000) {
    return `${Math.round(distance / 1000)} km`;
  }
  if (distance >= 1000) {
    const div = Math.round(distance / 1000);
    return `${div},${Math.round((distance - div) / 100)
      .toString()
      .slice(0, 1)} km`;
  }
  return `${Math.round(distance / 10) * 10} m`;
}
