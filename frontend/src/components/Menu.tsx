import '../styles/Menu.css';
import { MdTram } from 'react-icons/md';
import { AnimatePresence, motion } from 'framer-motion';

interface MenuProps {
  show: boolean;
  onHide: () => void;
}

export default function Menu({ show, onHide }: MenuProps) {
  return (
    <AnimatePresence>
      {show && (
        <motion.div
          initial={{ translateY: '-150%' }}
          animate={{ translateY: 0 }}
          exit={{ translateY: '-150%' }}
          className="Menu"
        >
          <div className="logo">
            <MdTram size={63} />
            <h1>Utforska Göteborgs kollektivtrafik</h1>
          </div>

          <div className="intro">
            Tryck på en hållplats för att få upp mer information om pendelvanor
            och kollektivtrafik-möjligheterna för personer som bor i närheten av
            hållplatsen.
          </div>
        </motion.div>
      )}
    </AnimatePresence>
  );
}
