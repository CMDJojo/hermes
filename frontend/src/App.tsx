import { useState } from 'react';

import Map from './components/Map';
import Sidebar from './components/Sidebar';
import Menu from './components/Menu';
import API, { InfoReport } from './api';
import Stop from './types/Stop';
import ActiveArea from './types/ActiveArea';

import './styles/App.css';

function App() {
  const [isSidebarActive, setIsSidebarActive] = useState<boolean>(false);
  const [activeStop, setActiveStop] = useState<Stop | null>(null);
  const [info, setInfo] = useState<InfoReport | null>(null);
  const [activeArea, setActiveArea] = useState<ActiveArea | null>(null);

  const api = new API();

  const updateSidebar = (stop: Stop) => {
    setActiveStop(stop);
    setIsSidebarActive(true);
    api
      .infoReport(stop.id.toString())
      .then(setInfo)
      .catch(() => setInfo(null));
  };

  const closeSidebar = () => {
    setIsSidebarActive(false);
    setActiveStop(null);
  };

  return (
    <div className="App">
      <Menu />
      <Map
        activeStop={activeStop}
        activeArea={activeArea}
        onClick={updateSidebar}
      />
      <Sidebar
        onClose={closeSidebar}
        active={isSidebarActive}
        stop={activeStop}
        info={info}
      />
    </div>
  );
}

export default App;
