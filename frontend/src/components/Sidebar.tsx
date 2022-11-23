import { MdClose } from 'react-icons/md';
import { motion, AnimatePresence } from 'framer-motion';
import { AreaChart, Area, XAxis, YAxis, ResponsiveContainer } from 'recharts';
import Stop from '../types/Stop';
import '../styles/Sidebar.css';
import InfoBox from './InfoBox';

interface SidebarProps {
  active: boolean;
  stop: Stop | null;
  onClose: () => void;
}

const data = [
  {
    name: 'Page A',
    uv: 4000,
    pv: 2400,
    amt: 2400,
  },
  {
    name: 'Page B',
    uv: 3000,
    pv: 1398,
    amt: 2210,
  },
  {
    name: 'Page C',
    uv: 2000,
    pv: 9800,
    amt: 2290,
  },
  {
    name: 'Page D',
    uv: 2780,
    pv: 3908,
    amt: 2000,
  },
  {
    name: 'Page E',
    uv: 1890,
    pv: 4800,
    amt: 2181,
  },
  {
    name: 'Page F',
    uv: 2390,
    pv: 3800,
    amt: 2500,
  },
  {
    name: 'Page G',
    uv: 3490,
    pv: 4300,
    amt: 2100,
  },
];

export default function Sidebar({ active, stop, onClose }: SidebarProps) {
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
            <h1>{stop.name}</h1>

            <div className="infoBoxes">
              <InfoBox color="#D7EBBA" title="Pendeltid till jobbet">
                <h1>30 min</h1>
                <ResponsiveContainer width="100%" height={150}>
                  <AreaChart
                    width={300}
                    height={150}
                    data={data}
                    margin={{
                      top: 0,
                      right: 0,
                      left: 0,
                      bottom: 0,
                    }}
                  >
                    <XAxis dataKey="name" />
                    <Area
                      type="monotone"
                      dataKey="uv"
                      stroke="#a8b791"
                      fill="#a8b791"
                    />
                    <YAxis />
                  </AreaChart>
                </ResponsiveContainer>
              </InfoBox>
              <InfoBox
                color="#9AD2CB"
                title="Hur många använder den här hållplatsen"
              >
                <h1>20%</h1>
              </InfoBox>
            </div>
          </div>
        </motion.div>
      )}
    </AnimatePresence>
  );
}
