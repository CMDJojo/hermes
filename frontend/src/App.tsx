import { useState } from 'react';

import Map from './components/Map';
import Sidebar from './components/Sidebar';

import './styles/App.css';

function App() {
  const [show, setShow] = useState(false);

  return (
    <div className="App">
      <div
        style={{
          position: 'absolute',
          zIndex: '1000',
          boxSizing: 'border-box',
          padding: '1rem',
        }}
      >
        <h1>Wow cool karta</h1>
        <button type="button" onClick={() => setShow(!show)}>
          Click plz {show ? 'true' : 'false'}
        </button>
      </div>
      <Map />
      <Sidebar>
        <h3>Test</h3>
      </Sidebar>
    </div>
  );
}

export default App;
