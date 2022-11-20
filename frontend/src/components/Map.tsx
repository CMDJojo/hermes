import { useRef, useState, useEffect, LegacyRef } from 'react';
import { Map as MapLibreGL } from 'maplibre-gl';

import '../styles/Map.css';

function Map() {
  const mapContainer = useRef<string | HTMLElement | null>(null);
  const map = useRef<MapLibreGL | null>(null);
  const [view] = useState({ lat: 57.7071367, lon: 11.9662978, zoom: 11 });

  useEffect(() => {
    if (map.current) return; // stops map from intializing more than once
    map.current = new MapLibreGL({
      container: mapContainer.current as string | HTMLElement,
      style: `https://api.maptiler.com/maps/streets-v2/style.json?key=${
        import.meta.env.VITE_MAPTILER_API_KEY
      }`,
      center: [view.lon, view.lat],
      zoom: view.zoom,
    });
  });

  return (
    <div className="Map">
      <div ref={mapContainer as LegacyRef<HTMLDivElement>} className="map" />
    </div>
  );
}

export default Map;
