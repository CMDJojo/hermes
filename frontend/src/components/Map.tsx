import { useRef, useState, useEffect, LegacyRef } from 'react';
import { Map as MapLibreGL } from 'maplibre-gl';
import { Feature, Geometry, GeoJsonProperties } from 'geojson';

import API, { Stops } from '../api';
import Stop from '../types/Stop';
import ActiveArea from '../types/ActiveArea';

import '../styles/Map.css';

export type StopEvent = Feature<Geometry, GeoJsonProperties>;
export interface MapProps {
  activeStop: Stop | null;
  activeArea: ActiveArea | null;
  onClick: (event: Stop) => void;
}

function Map({ onClick, activeStop }: MapProps) {
  const mapContainer = useRef<string | HTMLElement | null>(null);
  const api = useRef<API | null>(null);
  const map = useRef<MapLibreGL | null>(null);
  const [view] = useState({ lat: 57.7071367, lon: 11.9662978, zoom: 11 });

  useEffect(() => {
    if (api.current) return;

    api.current = new API();

    if (map.current) return; // stops map from intializing more than once
    map.current = new MapLibreGL({
      container: mapContainer.current as string | HTMLElement,
      style: `https://api.maptiler.com/maps/96d26bc0-1881-4822-a296-78dd505fb161/style.json?key=${
        import.meta.env.VITE_MAPTILER_API_KEY
      }`,
      center: [view.lon, view.lat],
      zoom: view.zoom,
    });

    // Add the stops
    api.current.stops().then(stops => {
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
              'icon-allow-overlap': true,
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
          const { features } = e as unknown as Stops;

          const {
            id,
            geometry: { coordinates },
            properties: { name },
          } = features[0];

          const [lon, lat] = coordinates;

          onClick({ id, name, lat, lon });

          map.current!.flyTo({ center: coordinates });
        });
      });
    });
  });

  // When a stop is selected fetch a extra gejson layer with information
  // about the relative travel time to every other stop
  useEffect(() => {
    if (map.current === null) return;
    if (activeStop === null) return;

    api.current?.relativeTravelTime(activeStop.id.toString()).then(data => {
      map.current?.addSource('relativeTravelTime', { type: 'geojson', data });
      map.current?.addLayer({
        id: 'relativeTravelTime',
        type: 'symbol',
        source: 'relativeTravelTime',
        layout: {
          'text-field': ['get', 'travelTime'],
          'text-font': ['Inter', 'Arial Unicode MS Bold'],
          'text-offset': [0, 3],
          'text-anchor': 'top',
          'text-size': 10,
        },
      });
    });
  }, [activeStop, api, map]);

  return (
    <div className="Map">
      <div ref={mapContainer as LegacyRef<HTMLDivElement>} className="map" />
    </div>
  );
}

export default Map;
