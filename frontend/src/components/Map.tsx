import { useRef, useState, useEffect, LegacyRef } from 'react';
import { Map as MapLibreGL, GeoJSONSource } from 'maplibre-gl';
import {
  Feature,
  Geometry,
  GeoJsonProperties,
  FeatureCollection,
} from 'geojson';

import API, { Stops } from '../api';
import Stop from '../types/Stop';
import ActiveArea from '../types/ActiveArea';

import '../styles/Map.css';

const EMPTY_GEOJSON_DATA: FeatureCollection = {
  type: 'FeatureCollection',
  features: [],
};

export type StopEvent = Feature<Geometry, GeoJsonProperties>;
export interface MapProps {
  activeStop: Stop | null;
  activeArea: ActiveArea | null;
  onClick: (event: Stop) => void;
}

// stackoverflow.com/questions/37599561/drawing-a-circle-with-the-radius-in-miles-meters-with-mapbox-gl-js
const createGeoJSONCircle = (
  center: [number, number],
  radiusInKm: number,
  points: number
): FeatureCollection => {
  if (!points) points = 64;

  const coords = {
    latitude: center[1],
    longitude: center[0],
  };

  const km = radiusInKm;

  const ret = [];
  const distanceX = km / (111.32 * Math.cos((coords.latitude * Math.PI) / 180));
  const distanceY = km / 110.574;

  let theta;
  let x;
  let y;
  for (let i = 0; i < points; i++) {
    theta = (i / points) * (2 * Math.PI);
    x = distanceX * Math.cos(theta);
    y = distanceY * Math.sin(theta);

    ret.push([coords.longitude + x, coords.latitude + y]);
  }
  ret.push(ret[0]);

  return {
    type: 'FeatureCollection',
    features: [
      {
        type: 'Feature',
        properties: {},
        geometry: {
          type: 'Polygon',
          coordinates: [ret],
        },
      },
    ],
  };
};

function Map({ onClick, activeStop }: MapProps) {
  const mapContainer = useRef<string | HTMLElement | null>(null);
  const api = useRef<API | null>(null);
  const map = useRef<MapLibreGL | null>(null);
  const [view] = useState({ lat: 57.7071367, lon: 11.9662978, zoom: 11 });

  const [mapLoaded, setMapLoaded] = useState(false);

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
              'icon-allow-overlap': false,
              'icon-size': 0.5,
              'text-field': ['get', 'name'],
              'text-font': ['Inter', 'Arial Unicode MS Bold'],
              'text-offset': [0, 1],
              'text-anchor': 'top',
              'text-size': 10,
            },
          });
        });

        // Create a data object and a layer  to store relative times from a active stop
        // Note that this must be populated with data later
        map.current?.addSource('relativeTravelTime', {
          type: 'geojson',
          data: EMPTY_GEOJSON_DATA,
        });

        // Same deal but a layer to display a circle on the active marker
        map.current?.addSource('activeCircle', {
          type: 'geojson',
          data: EMPTY_GEOJSON_DATA,
        });

        map.current?.addLayer({
          id: 'activeCircle',
          type: 'fill',
          source: 'activeCircle',
          layout: {},
          paint: {
            'fill-color': '#04aaeb',
            'fill-opacity': 0.2,
          },
        });

        map.current?.addLayer({
          id: 'relativeTravelTime',
          type: 'symbol',
          minzoom: 12,
          source: 'relativeTravelTime',
          layout: {
            'text-field': ['get', 'travelTime'],
            'text-font': ['Inter', 'Arial Unicode MS Bold'],
            'text-offset': [0, 2.8],
            'icon-allow-overlap': true,
            'text-anchor': 'top',
            'text-size': 10,
          },
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
        });

        setMapLoaded(true);
      });
    });
  });

  // Update the map when when a stop is selected or deselected
  useEffect(() => {
    if (map.current === null) return;
    if (!mapLoaded) return;

    // When the user deselects a stop, remove the data from the relativeTravelTime layer
    if (activeStop === null) {
      (map.current?.getSource('relativeTravelTime') as GeoJSONSource).setData(
        EMPTY_GEOJSON_DATA
      );

      (map.current?.getSource('activeCircle') as GeoJSONSource).setData(
        EMPTY_GEOJSON_DATA
      );
      return;
    }

    // When the user selects a stop
    map.current?.flyTo({ center: [activeStop.lon, activeStop.lat] });

    (map.current?.getSource('activeCircle') as GeoJSONSource).setData(
      createGeoJSONCircle([activeStop.lon, activeStop.lat], 0.5)
    );

    api.current?.relativeTravelTime(activeStop.id.toString()).then(data => {
      (map.current?.getSource('relativeTravelTime') as GeoJSONSource).setData(
        data as FeatureCollection
      );
    });
  }, [activeStop, api, map, mapLoaded]);

  return (
    <div className="Map">
      <div ref={mapContainer as LegacyRef<HTMLDivElement>} className="map" />
    </div>
  );
}

export default Map;
