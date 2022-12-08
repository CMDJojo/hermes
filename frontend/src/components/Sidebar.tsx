import { MdClose, MdExpandLess, MdExpandMore } from 'react-icons/md';
import { AnimatePresence, motion } from 'framer-motion';
import { useState } from 'react';
import {
  Bar,
  BarChart,
  ResponsiveContainer,
  Tooltip,
  TooltipProps,
  XAxis,
  YAxis,
} from 'recharts';
import {
  NameType,
  ValueType,
} from 'recharts/src/component/DefaultTooltipContent';
import Stop from '../types/Stop';
import '../styles/Sidebar.css';
import InfoBox from './InfoBox';
import { TravelDistance, TravelTime } from '../api';
import { formatDistance, formatPercent } from '../utils/format';

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
  const optimalPercent =
    timeInfo === null
      ? null
      : formatPercent(timeInfo.optimalNrPeople, timeInfo.peopleCanGoByBus);

  const filteredPeopleFrom =
    timeInfo != null
      ? timeInfo.peopleTravelFrom.filter(
          stat => stat.stopID !== timeInfo.interestingStopID
        )
      : null;

  const [displayAllStops, setDisplayAllStops] = useState(false);

  const defaultStopsToShow = 3;

  const toNumberLiteral = (num: number) => {
    if (num > 10 || num < 1) return num.toString();
    const arr = 'en,två,tre,fyra,fem,sex,sju,åtta,nio,tio'.split(',');
    return arr[num - 1];
  };

  const expansionText: () => string = () => {
    if (displayAllStops)
      return `Visa endast ${toNumberLiteral(defaultStopsToShow)}`;

    const overflow =
      (filteredPeopleFrom?.length ?? defaultStopsToShow) - defaultStopsToShow;
    return overflow > 1
      ? `Visa ytterligare ${toNumberLiteral(overflow)} alternativa hållplatser`
      : `Visa ytterligare en alternativ hållplats`;
  };

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
                      <XAxis
                        dataKey="name"
                        stroke="#284867"
                        axisLine={{ stroke: '#284867' }}
                      />
                      <Tooltip
                        cursor={{ fill: 'rgba(0,0,0,0.15)' }}
                        content={<TooltipContent />}
                      />
                      <YAxis
                        width={40}
                        stroke="#284867"
                        axisLine={{ stroke: '#284867' }}
                      />
                      <Bar dataKey="data" fill="#284867" />
                    </BarChart>
                  </ResponsiveContainer>
                </InfoBox>
              )}
              {timeInfo !== null && (
                <InfoBox
                  color="#6c8e79"
                  title="Medianrestid till arbete via kollektivtrafik"
                >
                  <h1>{timeInfo?.medianTravelTimeFormatted}</h1>
                  <ResponsiveContainer width="100%" height={150}>
                    <BarChart
                      width={800}
                      height={300}
                      data={timeInfo?.travelTimeStats}
                    >
                      <XAxis
                        dataKey="name"
                        stroke="#314137"
                        axisLine={{ stroke: '#314137' }}
                      />
                      <Tooltip
                        cursor={{ fill: 'rgba(0,0,0,0.15)' }}
                        content={<TooltipContent />}
                      />
                      <YAxis
                        width={40}
                        stroke="#314137"
                        axisLine={{ stroke: '#314137' }}
                      />
                      <Bar dataKey="data" fill="#314137" />
                    </BarChart>
                  </ResponsiveContainer>
                </InfoBox>
              )}

              {timeInfo !== null && (
                <InfoBox
                  title="Hur många kan åka kollektivt till sin arbetsplats?"
                  color="#28536B"
                >
                  <h1 style={{ display: 'inline' }}>
                    {formatPercent(
                      timeInfo.peopleCanGoByBus,
                      timeInfo.totalNrPeople
                    )}
                  </h1>
                </InfoBox>
              )}

              {timeInfo !== null && optimalPercent !== null && (
                <InfoBox
                  title="Hur många har den här som bästa hållplats på väg till arbetet?"
                  color="#8E6C88"
                >
                  <h1>{optimalPercent}</h1>
                  {filteredPeopleFrom !== null &&
                    filteredPeopleFrom.length > 0 && (
                      <div className="optimalStopListContainer">
                        Andra personer åker från
                        {filteredPeopleFrom
                          .map(stat => (
                            <div
                              key={stat.stopID}
                              className="optimalStopList"
                              style={{ opacity: displayAllStops ? 1 : 0.6 }}
                            >
                              <div>
                                <strong>{stat.stopName}</strong>
                              </div>
                              <div>
                                {formatPercent(
                                  stat.numberOfPersons,
                                  timeInfo.peopleCanGoByBus
                                )}
                              </div>
                            </div>
                          ))
                          .slice(0, displayAllStops ? 100 : 3)}
                        {filteredPeopleFrom.length > defaultStopsToShow && (
                          <button
                            type="button"
                            onClick={() => setDisplayAllStops(!displayAllStops)}
                          >
                            {expansionText()}{' '}
                            {displayAllStops ? (
                              <MdExpandLess size={20} />
                            ) : (
                              <MdExpandMore size={20} />
                            )}
                          </button>
                        )}
                      </div>
                    )}
                </InfoBox>
              )}

              <InfoBox title="Debug" color="#FA6607" textColor="black">
                Stop ID: {stop.id}
                <br />
                Time info: {timeInfo === null ? 'null' : 'non-null'}
                <br />
                Distance info null:{' '}
                {distanceInfo === null ? 'null' : 'non-null'}
              </InfoBox>
            </div>
          </div>
        </motion.div>
      )}
    </AnimatePresence>
  );
}
