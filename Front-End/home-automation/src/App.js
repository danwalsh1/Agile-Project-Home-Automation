import React from 'react';
import logo from './logo.svg';
import './App.css';
import Websocket from 'react-websocket';
import 'bootstrap/dist/css/bootstrap.min.css';
import LightControl from './componenets/lightControl';

function handleWebsocketData(data){
  //let result = JSON.parse(data);
  console.log(data);
}

function App() {
  return (
    <div className="App">
      <Websocket url='localhost:8888/live' onMessage={handleWebsocketData} />
      <div>
        <LightControl />
      </div>
    </div>
  );
}

export default App;
