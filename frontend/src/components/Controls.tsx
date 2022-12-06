import '../styles/Controls.css';
import { AnimatePresence, motion } from 'framer-motion';

interface ControlsProps {
  show: boolean;
}

export default function Controls({ show }: ControlsProps) {
  return (
    <AnimatePresence>
      {show && (
        <motion.div
          initial={{ translateY: '150%' }}
          animate={{ translateY: 0 }}
          exit={{ translateY: '150%' }}
          transition={{ delay: 0.5 }}
          className="Controls"
        >
          <strong>Avancerade alternativ</strong>
          <div className="container">
            <div className="timetable">
              <div>
                <span>Restid och datum</span>
                <input
                  type="datetime-local"
                  id="meeting-time"
                  name="meeting-time"
                  value="2018-06-12T19:30"
                  min="2018-06-07T00:00"
                  max="2018-06-14T00:00"
                />
              </div>
              <div>
                <span>Tidtabell</span>
                <select name="pets" id="pet-select">
                  <option value="">--Please choose an option--</option>
                  <option value="dog">Dog</option>
                  <option value="cat">Cat</option>
                  <option value="hamster">Hamster</option>
                  <option value="parrot">Parrot</option>
                  <option value="spider">Spider</option>
                  <option value="goldfish">Goldfish</option>
                </select>
              </div>
            </div>
            <div className="other">Visa trafiklinjer i kartan</div>
          </div>
        </motion.div>
      )}
    </AnimatePresence>
  );
}
