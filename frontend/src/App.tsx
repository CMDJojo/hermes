import { useState } from 'react';

import Map from './components/Map';
import Sidebar from './components/Sidebar';
import Menu from './components/Menu';

import './styles/App.css';

function App() {
  return (
    <div className="App">
      <Menu />
      <Map />
      <Sidebar>
        <h3>Test</h3>
      </Sidebar>
    </div>
  );
}

export default App;
