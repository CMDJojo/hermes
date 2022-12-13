import { MdClose, MdExpandLess, MdExpandMore } from 'react-icons/md';
import { AnimatePresence, motion } from 'framer-motion';
import { useEffect, useState } from 'react';
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
import { formatDistance, formatPercent, formatTime } from '../utils/format';

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
  const [displayAllTransferStops, setDisplayAllTransferStops] = useState(false);

  const defaultStopsToShow = 3;

  useEffect(() => setDisplayAllStops(false), [timeInfo]);

  const toNumberLiteral = (num: number) => {
    const arr = 'en,två,tre,fyra,fem,sex,sju,åtta,nio,tio,elva,tolv'.split(',');
    if (num > arr.length || num < 1) return num.toString();
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

  const expansionTextTransfers: () => string = () => {
    if (displayAllTransferStops)
      return `Visa endast ${toNumberLiteral(defaultStopsToShow)}`;

    const overflow =
      (timeInfo?.transfers.length ?? defaultStopsToShow) - defaultStopsToShow;
    return overflow > 1
      ? `Visa ytterligare ${toNumberLiteral(overflow)} hållplatser`
      : `Visa ytterligare en hållplats`;
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
                <InfoBox color="#6c8e79" title="Medianavstånd till arbete">
                  <h1>{formatDistance(distanceInfo?.medianDistance)}</h1>
                  <ResponsiveContainer width="100%" height={150}>
                    <BarChart
                      width={800}
                      height={300}
                      data={distanceInfo?.distanceStats}
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
                  color="#84ACCE"
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

              {timeInfo !== null && (
                <InfoBox
                  title="Medelantalet byten per resa till arbetet"
                  color="#CEA07E"
                >
                  <h1>
                    {(timeInfo.numberOfTransfers / timeInfo.peopleCanGoByBus)
                      .toFixed(2)
                      .replace('.', ',')}
                  </h1>
                  {timeInfo.transfers !== null &&
                    timeInfo.transfers.length > 0 && (
                      <div className="optimalStopListContainer">
                        Vanligaste hållplatserna för byten
                        {timeInfo.transfers
                          .map(stat => (
                            <div
                              key={stat.stopID}
                              className="optimalStopList"
                              style={{
                                opacity: displayAllTransferStops ? 1 : 0.6,
                              }}
                            >
                              <div>
                                <strong>{stat.stopName}</strong>
                              </div>
                              <div>{formatPercent(stat.percentage, 100)}</div>
                            </div>
                          ))
                          .slice(0, displayAllTransferStops ? 100 : 3)}
                        {timeInfo.transfers.length > defaultStopsToShow && (
                          <button
                            type="button"
                            onClick={() =>
                              setDisplayAllTransferStops(
                                !displayAllTransferStops
                              )
                            }
                          >
                            {expansionTextTransfers()}{' '}
                            {displayAllTransferStops ? (
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

              {timeInfo !== null && timeInfo.distStopsFrom.length > 0 && (
                <InfoBox
                  color="#B84E5C"
                  title="Antal hållplatser personerna inom cirkeln bor nära"
                >
                  <h1>{timeInfo.avgStopsFrom.toFixed(2).replace('.', ',')}</h1>
                  <ResponsiveContainer width="100%" height={150}>
                    <BarChart
                      width={800}
                      height={300}
                      data={timeInfo.distStopsFrom}
                    >
                      <XAxis
                        dataKey="name"
                        stroke="#602830"
                        axisLine={{ stroke: '#602830' }}
                      />
                      <Tooltip
                        cursor={{ fill: 'rgba(0,0,0,0.15)' }}
                        content={<TooltipContent />}
                      />
                      <YAxis
                        width={40}
                        stroke="#602830"
                        axisLine={{ stroke: '#602830' }}
                      />
                      <Bar dataKey="data" fill="#602830" />
                    </BarChart>
                  </ResponsiveContainer>
                </InfoBox>
              )}

              {timeInfo !== null && timeInfo.distStopsTo.length > 0 && (
                <InfoBox
                  color="#EEB902"
                  title="Antal hållplatser personerna inom cirkeln har nära sitt jobb"
                >
                  <h1>{timeInfo.avgStopsTo.toFixed(2).replace('.', ',')}</h1>
                  <ResponsiveContainer width="100%" height={150}>
                    <BarChart
                      width={800}
                      height={300}
                      data={timeInfo.distStopsTo}
                    >
                      <XAxis
                        dataKey="name"
                        stroke="#644E02"
                        axisLine={{ stroke: '#644E02' }}
                      />
                      <Tooltip
                        cursor={{ fill: 'rgba(0,0,0,0.15)' }}
                        content={<TooltipContent />}
                      />
                      <YAxis
                        width={40}
                        stroke="#644E02"
                        axisLine={{ stroke: '#644E02' }}
                      />
                      <Bar dataKey="data" fill="#644E02" />
                    </BarChart>
                  </ResponsiveContainer>
                </InfoBox>
              )}

              {distanceInfo !== null && (
                <InfoBox title="Hållplatsinformation" color="#84ACCE">
                  {distanceInfo.boardings != null &&
                    `Dagliga påstigningar: ${distanceInfo.boardings?.toLocaleString()}`}
                  {distanceInfo.boardings != null && <br />}
                  Bytesmarginal: {formatTime(distanceInfo.minTransferTime)}
                </InfoBox>
              )}

              <InfoBox title="Debug" color="#eeeeee" textColor="black">
                Stop ID: {stop.id}
                <br />
                Time info: {timeInfo === null ? 'null' : 'not null'}
                <br />
                Distance info: {distanceInfo === null ? 'null' : 'not null'}
              </InfoBox>
            </div>
          </div>
        </motion.div>
      )}
    </AnimatePresence>
  );
}
