import { useState } from 'react';

import Map, { DetailInfoData } from './components/Map';
import Sidebar from './components/Sidebar';
import Menu from './components/Menu';
import DetailInfo from './components/DetailInfo';
import API, { TravelDistance, TravelTime } from './api';
import Stop from './types/Stop';
import Controls from './components/Controls';

import './styles/App.css';

function App() {
  const [isSidebarActive, setIsSidebarActive] = useState<boolean>(false);
  const [activeStop, setActiveStop] = useState<Stop | null>(null);
  const [distanceInfo, setDistanceInfo] = useState<TravelDistance | null>(null);
  const [timeInfo, setTimeInfo] = useState<TravelTime | null>(null);
  const [detailInfoData, setDetailedInfo] = useState<DetailInfoData | null>(
    null
  );

  const [showMenu, setShowMenu] = useState<boolean>(true);

  const api = new API();

  const updateSidebar = (stop: Stop) => {
    setActiveStop(stop);
    setIsSidebarActive(true);

    api
      .travelDistance(stop.id.toString())
      .then(setDistanceInfo)
      .catch(() => setDistanceInfo(null));

    api
      .travelTime(stop.id.toString())
      .then(setTimeInfo)
      .catch(() => setTimeInfo(null));
  };

  const closeSidebar = () => {
    setIsSidebarActive(false);
    setActiveStop(null);
  };

  return (
    <div className="App">
      <Menu show={showMenu} onHide={() => setShowMenu(false)} />
      <DetailInfo info={detailInfoData} />
      <Controls show={!showMenu} />
      <Map
        activeStop={activeStop}
        activeLines={timeInfo?.geojson ?? null}
        onClick={updateSidebar}
        onInteract={() => setShowMenu(false)}
        onShowDetailInfo={setDetailedInfo}
        onHideDetailInfo={() => setDetailedInfo(null)}
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
