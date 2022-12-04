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
import { TravelDistance, TravelTime } from '../api';
import formatDistance from '../utils/format';

interface SidebarProps {
  active: boolean;
  stop: Stop | null;
  distanceInfo: TravelDistance | null;
  timeInfo: TravelTime | null;
  onClose: () => void;
}

export default function Sidebar({
  active,
  stop,
  distanceInfo,
  timeInfo,
  onClose,
}: SidebarProps) {
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
            {distanceInfo !== null && (
              <strong>
                Boende inom {formatDistance(distanceInfo?.peopleRange)}:
                {distanceInfo?.nrPeople}
              </strong>
            )}
            <div className="infoBoxes">
              {distanceInfo !== null && (
                <InfoBox color="#84ACCE" title="Medianavstånd till arbete">
                  <h1>{formatDistance(distanceInfo?.medianDistance)}</h1>
                  <ResponsiveContainer width="100%" height={150}>
                    <BarChart
                      width={800}
                      height={300}
                      data={distanceInfo?.distanceStats}
                    >
                      <XAxis dataKey="name" />
                      <Tooltip />
                      <YAxis /* tickFormatter={tick => `${tick}%`} */ />
                      <Bar dataKey="distance" fill="#284867" />
                    </BarChart>
                  </ResponsiveContainer>
                </InfoBox>
              )}
              {timeInfo !== null && (
                <InfoBox title="Medianrestid till arbetet" color="#6c8e79">
                  <h1>{timeInfo.medianTravelTimeFormatted}</h1>
                </InfoBox>
              )}
              {timeInfo !== null && (
                <InfoBox
                  title="Hur många har den här som bästa hållplats på väg till arbetet?"
                  color="#8E6C88"
                >
                  <h1>
                    {Math.round(
                      (timeInfo.optimalNrPeople / timeInfo.totalNrPeople) * 100
                    )}
                    %
                  </h1>
                </InfoBox>
              )}
              <InfoBox title="Debug" color="#eee" textColor="black">
                Stop ID: {stop.id}
              </InfoBox>
            </div>
          </div>
        </motion.div>
      )}
    </AnimatePresence>
  );
}
