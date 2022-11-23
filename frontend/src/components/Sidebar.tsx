import { MdClose } from 'react-icons/md';
import { motion, AnimatePresence } from 'framer-motion';

import Stop from '../types/Stop';

import '../styles/Sidebar.css';

interface SidebarProps {
  active: boolean;
  stop: Stop | null;
  onClose: () => void;
}

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
            <h2>{stop.name}</h2>
            {stop.id}
          </div>
        </motion.div>
      )}
    </AnimatePresence>
  );
}
