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
import API, { InfoReport } from '../api';

interface SidebarProps {
  active: boolean;
  stop: Stop | null;
  onClose: () => void;
}

export default function Sidebar({ active, stop, onClose }: SidebarProps) {
  const [info, setInfo] = useState<InfoReport | null>(null);

  // load the info report about the stop
  useEffect(() => {
    if (stop === null) return;

    const api = new API();
    api
      .infoReport(`${stop.id}`)
      .then(setInfo)
      .catch(() => setInfo(null));
  }, [stop]);

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
              <strong>Loading data from server...</strong>
            ) : (
              <>
                <strong>People living within {info?.peopleRange}m: </strong>
                {info?.nrPeople}
                <div className="infoBoxes">
                  <InfoBox
                    color="#D7EBBA"
                    title="Genomsnittligt avstånd till jobbet"
                  >
                    <h1>{info?.avgDistance} meter</h1>
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
