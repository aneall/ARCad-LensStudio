function setInterval(callback, interval, executeInitially) {
  var delay = script.createEvent("DelayedCallbackEvent");
  delay.bind(()=>{
    callback(); delay.reset(interval);
  });
  if (executeInitially)
  {
    delay.reset(0.0001);
  }
  else
  {
    delay.reset(interval);
  }
  return delay;
}

global.setInterval = setInterval