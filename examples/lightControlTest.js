const lightsON = async () => {
    const response = await fetch('https://api.particle.io/v1/devices/e00fce681c2671fc7b1680eb/lightControl', {
      method: 'POST',
      body: 'access_token=79d528c35795f9a36a5e43e99105083d51cd87ac&args=1', // string or object
      headers: {
        'Content-Type': 'application/x-www-form-urlencoded'
      }
    });
    const myJson = await response.json(); //extract JSON from the http response
    // do something with myJson
    console.log(myJson)
  }

  const lightsOFF = async () => {
    const response = await fetch('https://api.particle.io/v1/devices/e00fce681c2671fc7b1680eb/lightControl', {
      method: 'POST',
      body: 'access_token=79d528c35795f9a36a5e43e99105083d51cd87ac&args=0', // string or object
      headers: {
        'Content-Type': 'application/x-www-form-urlencoded'
      }
    });
    const myJson = await response.json(); //extract JSON from the http response
    // do something with myJson
    console.log(myJson)
  }

