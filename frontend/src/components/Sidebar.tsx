import {useEffect, useState} from 'react';
import {MdClose} from 'react-icons/md';
import {motion, AnimatePresence} from 'framer-motion';
import {
  Bar,
  BarChart,
  Tooltip,
  XAxis,
  YAxis,
  ResponsiveContainer,
} from 'recharts';
import Stop from '../types/Stop';
import '../styles/Sidebar.css';
import InfoBox from './InfoBox';
import API, {InfoReport} from '../api';

interface SidebarProps {
  active: boolean;
  stop: Stop | null;
  onClose: () => void;
}

export default function Sidebar({active, stop, onClose}: SidebarProps) {
  const [info, setInfo] = useState<InfoReport | null>(null);

  // load the info report about the stop
  useEffect(() => {
    if (stop === null) return;

    const api = new API();
    setInfo(null);
    api
      .infoReport(`${stop.id}`)
      .then(setInfo)
      .catch(() => setInfo(null));
  }, [stop]);

  return (
    <AnimatePresence>
      {active && stop !== null && (
        <motion.div
          initial={{translateX: '120%', translateY: -50}}
          animate={{translateX: 0, translateY: 0}}
          exit={{translateX: '120%', translateY: 50}}
          className="Sidebar"
        >
          <button onClick={onClose} type="button" className="closeButton">
            <MdClose className="closeIcon" size="1.5rem"/>
          </button>
          <div className="content">
            <h1>{stop.name}</h1>
            Stop ID: {stop.id}
            <br/>
            {info === null ? <strong>Loading data from server...</strong> : (
              <>
                <strong>avg distance:</strong> {info?.avgDistance}
                <br/>
                <br/>
                <strong>nr. people:</strong> {info?.nrPeople}
                <div className="infoBoxes">
                  <InfoBox color="#D7EBBA" title="Avstånd till jobbet">
                    <ResponsiveContainer width="100%" height={150}>
                      <BarChart width={800} height={300} data={info?.distanceStats}>
                        <XAxis dataKey="name"/>
                        <Tooltip/>
                        <YAxis /* tickFormatter={tick => `${tick}%`} */ />
                        <Bar dataKey="distance" fill="#8884d8"/>
                      </BarChart>
                    </ResponsiveContainer>
                  </InfoBox>
                  <InfoBox
                    color="#9AD2CB"
                    title="Hur många använder den här hållplatsen"
                  >
                    <h1>20%</h1>
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
