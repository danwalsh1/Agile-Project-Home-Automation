import React from 'react';
import logo from './logo.svg';
import './App.css';
import 'bootstrap/dist/css/bootstrap.min.css';
import LightControl from './componenets/lightControl';

function App() {
  return (
    <div className="App">
      <div>
        <LightControl />
      </div>
    </div>
  );
}

export default App;
