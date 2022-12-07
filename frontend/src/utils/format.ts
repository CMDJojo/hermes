export function formatDistance(distance: number): string {
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

export function formatPercent(
  numerator: number,
  denomenator: number,
  decimals = 0
): string {
  if (numerator === 0) return '0%';
  if (denomenator === 0 || Number.isNaN(numerator) || Number.isNaN(numerator))
    return 'N/A';
  const fraction = numerator / denomenator;
  const factor = 10 ** decimals;
  const rounded = Math.round(fraction * 100 * factor);
  const divided = rounded / factor;
  return `${divided}%`;
}
