import { useRef, useState, useEffect, LegacyRef } from 'react';
import { Map as MapLibreGL } from 'maplibre-gl';
import { Point, Feature, Geometry, GeoJsonProperties } from 'geojson';

import { getStops } from '../api';
import Stop from '../types/Stop';

import '../styles/Map.css';

type Pair<T> = [T, T];

export type StopEvent = Feature<Geometry, GeoJsonProperties>;
export interface MapProps {
  onClick: (event: Stop) => void;
}

function Map({ onClick }: MapProps) {
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

    // Add the stops
    getStops().then(stops => {
      map.current?.on('load', () => {
        map.current?.loadImage('./pin.png', (error, image) => {
          if (error) throw error;
          map.current?.addImage('custom-marker', image!);

          map.current?.addSource('stops', { type: 'geojson', data: stops });
          map.current?.addLayer({
            id: 'stops',
            type: 'symbol',
            source: 'stops',
            layout: {
              'icon-image': 'custom-marker',
              'icon-size': 0.5,
              'text-field': ['get', 'name'],
              'text-font': ['Inter', 'Arial Unicode MS Bold'],
              'text-offset': [0, 1],
              'text-anchor': 'top',
              'text-size': 10,
            },
          });
        });

        // Change the cursor to a pointer when the mouse is over the places layer.
        map.current?.on('mouseenter', 'stops', () => {
          map.current!.getCanvas().style.cursor = 'pointer';
        });

        // Change it back to a pointer when it leaves.
        map.current?.on('mouseleave', 'stops', () => {
          map.current!.getCanvas().style.cursor = '';
        });

        // Center the map on the coordinates of any clicked symbol from the 'symbols' layer.
        map.current?.on('click', 'stops', e => {
          const stopData = e?.features![0];

          const [lon, lat]: [number, number] = (stopData.geometry as Point)
            ?.coordinates as Pair<number>;

          onClick({
            id: stopData.id as number,
            name: stopData.properties?.name,
            lat,
            lon,
          });
          map.current!.flyTo({
            center: (e?.features?.[0]?.geometry as Point)
              ?.coordinates as Pair<number>,
          });
        });
      });
    });
  });

  return (
    <div className="Map">
      <div ref={mapContainer as LegacyRef<HTMLDivElement>} className="map" />
    </div>
  );
}

export default Map;
