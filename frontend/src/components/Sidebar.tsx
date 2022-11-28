import { useEffect, useState } from 'react';
import { MdClose } from 'react-icons/md';
import { AnimatePresence, motion } from 'framer-motion';
import {
  Bar,
  BarChart,
  ResponsiveContainer,
  Tooltip,
  XAxis,
  YAxis,
} from 'recharts';
import Stop from '../types/Stop';
import '../styles/Sidebar.css';
import InfoBox from './InfoBox';
import { InfoReport } from '../api';

interface SidebarProps {
  active: boolean;
  stop: Stop | null;
  info: InfoReport | null;
  onClose: () => void;
}

export default function Sidebar({ active, stop, info, onClose }: SidebarProps) {
  return (
    <AnimatePresence>
      {active && stop !== null && (
        <motion.div
          initial={{ translateX: '120%', translateY: -50 }}
          animate={{ translateX: 0, translateY: 0 }}
          exit={{ translateX: '120%', translateY: 50 }}
          className="Sidebar"
        >
          <button onClick={onClose} type="button" className="closeButton">
            <MdClose className="closeIcon" size="1.5rem" />
          </button>
          <div className="content">
            <h1 className="heading">{stop.name}</h1>
            <span className="" />
            {info === null ? (
              <strong>Failed to load data</strong>
            ) : (
              <>
                <strong>
                  People living within {info?.peopleRange}m: {info?.nrPeople}{' '}
                </strong>
                <div className="infoBoxes">
                  <InfoBox
                    color="#D7EBBA"
                    title="Genomsnittligt avstånd till jobbet"
                  >
                    <h1>{info?.medianDistance} meter</h1>
                    <ResponsiveContainer width="100%" height={150}>
                      <BarChart
                        width={800}
                        height={300}
                        data={info?.distanceStats}
                      >
                        <XAxis dataKey="name" />
                        <Tooltip />
                        <YAxis /* tickFormatter={tick => `${tick}%`} */ />
                        <Bar dataKey="distance" fill="#8884d8" />
                      </BarChart>
                    </ResponsiveContainer>
                  </InfoBox>
                  <InfoBox title="Debug" color="#eee">
                    Stop ID: {stop.id}
                  </InfoBox>
                </div>
              </>
            )}
          </div>
        </motion.div>
      )}
    </AnimatePresence>
  );
}
