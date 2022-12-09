import '../styles/Controls.css';
import { AnimatePresence, motion } from 'framer-motion';
import DateTimePicker from 'react-datetime-picker';
import { Timetable } from '../api';

interface ControlsProps {
  show: boolean;
  timetables: Timetable[];
  currentTimetable: Timetable | null;
}

export default function Controls({
  show,
  timetables,
  currentTimetable,
}: ControlsProps) {
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
          <strong>Avancerade alternativ längd:</strong>
          <div className="container">
            <div className="timetable">
              <div>
                <span>Restid och datum</span>
                <DateTimePicker
                  minDate={
                    new Date(
                      currentTimetable?.startDate.year ?? 2022,
                      currentTimetable?.startDate.month ?? 10,
                      currentTimetable?.startDate.day ?? 6,
                      8,
                      0
                    )
                  }
                  value={
                    new Date(
                      currentTimetable?.startDate.year ?? 2022,
                      currentTimetable?.startDate.month ?? 10,
                      currentTimetable?.startDate.day ?? 6,
                      8,
                      0
                    )
                  }
                  maxDate={
                    new Date(
                      currentTimetable?.endDate.year ?? 2022,
                      currentTimetable?.endDate.month ?? 10,
                      currentTimetable?.endDate.day ?? 6,
                      8,
                      0
                    )
                  }
                />
              </div>
              <div>
                <span>Tidtabell</span>
                <select>
                  {timetables.map(t => (
                    <option value={t.id} key={t.id}>
                      {t.name}
                    </option>
                  ))}
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
