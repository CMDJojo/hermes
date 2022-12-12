import { useState, useEffect } from 'react';

import Map, { DetailInfoData } from './components/Map';
import Sidebar from './components/Sidebar';
import Menu from './components/Menu';
import DetailInfo from './components/DetailInfo';
import API, { TravelDistance, TravelTime, Timetable } from './api';
import Stop from './types/Stop';
import Controls from './components/Controls';

import './styles/App.css';

function App() {
  const [isSidebarActive, setIsSidebarActive] = useState<boolean>(false);
  const [activeStop, setActiveStop] = useState<Stop | null>(null);
  const [distanceInfo, setDistanceInfo] = useState<TravelDistance | null>(null);
  const [timeInfo, setTimeInfo] = useState<TravelTime | null>(null);
  const [timetables, setTimetables] = useState<Timetable[]>([]);
  const [currentTimetable, setCurrentTimetable] = useState<Timetable | null>(
    null
  );
  const [detailInfoData, setDetailedInfo] = useState<DetailInfoData | null>(
    null
  );

  const [showTrafficLines, setShowTrafficLines] = useState(true);

  const [showMenu, setShowMenu] = useState<boolean>(true);

  const [date, setDate] = useState<Date | null>(null);

  // Fetch the timetable and set the time and currently used timetable
  useEffect(() => {
    const api = new API();

    api
      .timetables()
      .then(data => {
        setTimetables(data);
        if (data.length > 0) {
          const minDate =
            currentTimetable !== null
              ? new Date(
                  data[0].startDate.year,
                  data[0].startDate.month - 1,
                  data[0].startDate.day,
                  8,
                  0
                )
              : new Date();
          setCurrentTimetable(data[0]);
          setDate(minDate);
        }
      })
      .catch(() => setTimetables([]));
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  // GEt new data for the sidebar
  useEffect(() => {
    const api = new API();

    if (activeStop === null) return;

    const options = {
      date: date ?? undefined,
      timetableId: currentTimetable?.id ?? undefined,
    };

    api
      .travelDistance(activeStop.id.toString(), options)
      .then(setDistanceInfo)
      .catch(() => setDistanceInfo(null));

    api
      .travelTime(activeStop.id.toString(), options)
      .then(setTimeInfo)
      .catch(() => setTimeInfo(null));
  }, [activeStop, currentTimetable, date]);

  const updateSidebar = (stop: Stop) => {
    setActiveStop(stop);
    setIsSidebarActive(true);
  };

  const closeSidebar = () => {
    setIsSidebarActive(false);
    setActiveStop(null);
  };

  return (
    <div className="App">
      <Menu show={showMenu} onHide={() => setShowMenu(false)} />
      <DetailInfo info={detailInfoData} />
      {timetables.length > 0 && date !== null && (
        <Controls
          currentTimetable={currentTimetable}
          currentDate={date}
          show={!showMenu}
          onDateChange={setDate}
          onTimetableChange={setCurrentTimetable}
          timetables={timetables}
          showTrafficLines={showTrafficLines}
          toggleTrafficLines={() => setShowTrafficLines(!showTrafficLines)}
        />
      )}
      <Map
        activeStop={activeStop}
        showTrafficLines={showTrafficLines}
        activeLines={timeInfo?.lines ?? null}
        activeWalks={timeInfo?.walks ?? null}
        onClick={stop => {
          updateSidebar(stop);
          setShowMenu(false);
        }}
        onShowDetailInfo={setDetailedInfo}
        onHideDetailInfo={() => setDetailedInfo(null)}
        date={date}
        timetable={currentTimetable}
      />
      <Sidebar
        onClose={closeSidebar}
        active={isSidebarActive}
        stop={activeStop}
        distanceInfo={distanceInfo}
        timeInfo={timeInfo}
      />
    </div>
  );
}

export default App;
