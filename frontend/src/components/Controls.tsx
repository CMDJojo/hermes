import '../styles/Controls.css';
import { AnimatePresence, motion } from 'framer-motion';
import DatePicker, { registerLocale } from 'react-datepicker';
import 'react-datepicker/dist/react-datepicker.css';
import sv from 'date-fns/locale/sv';
import { useState } from 'react';
import { Timetable } from '../api';

registerLocale('sv', sv);

interface ControlsProps {
  show: boolean;
  timetables: Timetable[];
  currentTimetable: Timetable | null;
  currentDate: Date;
  onDateChange: (date: Date) => void;
  showTrafficLines: boolean;
  toggleTrafficLines: () => void;
}

export default function Controls({
  show,
  timetables,
  currentTimetable,
  onDateChange,
  currentDate,
  showTrafficLines,
  toggleTrafficLines,
}: ControlsProps) {
  const minDate =
    currentTimetable !== null
      ? new Date(
          currentTimetable.startDate.year,
          currentTimetable.startDate.month - 1,
          currentTimetable.startDate.day,
          8,
          0
        )
      : new Date();

  const maxDate =
    currentTimetable !== null
      ? new Date(
          currentTimetable.endDate.year,
          currentTimetable.endDate.month - 1,
          currentTimetable.endDate.day,
          8,
          0
        )
      : new Date();

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
          <div className="container">
            <div className="timetable">
              <div>
                <span>Restid och datum</span>
                <div>
                  <DatePicker
                    selected={currentDate}
                    minDate={minDate}
                    maxDate={maxDate}
                    onChange={d => {
                      if (d !== null) onDateChange(d);
                    }}
                    locale="sv"
                    showTimeSelect
                    timeFormat="p"
                    timeIntervals={15}
                    dateFormat="Pp"
                  />
                </div>
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
            <button
              onClick={toggleTrafficLines}
              type="button"
              className="traficLines"
            >
              <img src="map_icon.png" alt="icon" />
              <span>{showTrafficLines ? 'Dölj' : 'Visa'} trafiklinjer</span>
            </button>
          </div>
        </motion.div>
      )}
    </AnimatePresence>
  );
}
