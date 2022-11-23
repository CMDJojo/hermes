import { useState } from 'react';

import Map from './components/Map';
import Sidebar from './components/Sidebar';
import Menu from './components/Menu';
import Stop from './types/Stop';

import './styles/App.css';

function App() {
  const [isSidebarActive, setIsSidebarActive] = useState<boolean>(false);
  const [activeStop, setActiveStop] = useState<Stop | null>(null);

  const updateSidebar = (stop: Stop) => {
    setActiveStop(stop);
    setIsSidebarActive(true);
  };

  return (
    <div className="App">
      <Menu />
      <Map onClick={updateSidebar} />
      <Sidebar
        onClose={() => setIsSidebarActive(false)}
        active={isSidebarActive}
        stop={activeStop}
      />
    </div>
  );
}

export default App;
