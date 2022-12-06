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
  TooltipProps,
} from 'recharts';
import {
  ValueType,
  NameType,
} from 'recharts/src/component/DefaultTooltipContent';
import Stop from '../types/Stop';
import '../styles/Sidebar.css';
import InfoBox from './InfoBox';
import { TravelDistance, TravelTime } from '../api';
import formatDistance from '../utils/format';

function TooltipContent({
  active,
  payload,
  label,
}: TooltipProps<ValueType, NameType>) {
  const { data } = payload?.[0]?.payload || { data: null };
  return (
    <div className="TooltipContent">
      {active && payload!.length > 0 && (
        <>
          <p className="label">{label}</p>
          <p className="data">Personer: {data}</p>
        </>
      )}
    </div>
  );
}

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
  const optimalDiv =
    timeInfo !== null
      ? timeInfo.optimalNrPeople / timeInfo.totalNrPeople
      : null;
  const optimalPercent =
    timeInfo !== null &&
    !Number.isNaN(optimalDiv) &&
    Number.isFinite(optimalDiv)
      ? Math.round(optimalDiv! * 100)
      : null;
  return (
    <AnimatePresence>
      {active && stop !== null && (
        <motion.div
          initial={{ translateX: '120%', translateY: -50 }}
          animate={{ translateX: 0, translateY: 0 }}
          exit={{ translateX: '120%', translateY: 50 }}
          className="Sidebar"
        >
          <div className="header">
            <h1 className="heading">{stop.name}</h1>
            <button onClick={onClose} type="button" className="closeButton">
              <MdClose className="closeIcon" size="1.5rem" />
            </button>
          </div>
          <div className="content">
            {distanceInfo !== null && (
              <span className="nrPeople">
                Här finns data för {distanceInfo?.nrPeople} personer inom en{' '}
                {formatDistance(distanceInfo?.peopleRange)} radie.
              </span>
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
                      <Tooltip
                        cursor={{ fill: 'rgba(0,0,0,0.15)' }}
                        content={<TooltipContent />}
                      />
                      <YAxis
                        width={40} /* tickFormatter={tick => `${tick}%`} */
                      />
                      <Bar dataKey="data" fill="#284867" />
                    </BarChart>
                  </ResponsiveContainer>
                </InfoBox>
              )}
              {timeInfo !== null && (
                <InfoBox color="#6c8e79" title="Medianrestid till arbete">
                  <h1>{timeInfo?.medianTravelTimeFormatted}</h1>
                  <ResponsiveContainer width="100%" height={150}>
                    <BarChart
                      width={800}
                      height={300}
                      data={timeInfo?.travelTimeStats}
                    >
                      <XAxis dataKey="name" />
                      <Tooltip
                        cursor={{ fill: 'rgba(0,0,0,0.15)' }}
                        content={<TooltipContent />}
                      />
                      <YAxis width={40} />
                      <Bar dataKey="data" fill="#314137" />
                    </BarChart>
                  </ResponsiveContainer>
                </InfoBox>
              )}
              {timeInfo !== null && optimalPercent !== null && (
                <InfoBox
                  title="Hur många har den här som bästa hållplats på väg till arbetet?"
                  color="#8E6C88"
                >
                  <h1>{optimalPercent}%</h1>
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
