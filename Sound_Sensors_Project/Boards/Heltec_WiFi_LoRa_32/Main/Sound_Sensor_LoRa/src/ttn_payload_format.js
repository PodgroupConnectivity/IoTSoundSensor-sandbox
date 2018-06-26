// Function to decode payload sent to The Things Network
// This goes under https://console.thethingsnetwork.org/
// Applications > <your_app> > Payload Formats

// This take the payload and translate to a JSON with
// 3 parameters: max sound, min sound, average sound

function Decoder(bytes, port) {
  
  // Decode payload as string
  var dBA_string = String.fromCharCode.apply(null, bytes);
  dBA_string = dBA_string.split('\00').join('');
  dBA_string = dBA_string.split(';');
  
  
  // Parsing to a float
  return {
    //dBA: parseFloat(dBA_string
    maximum_dBA: parseFloat(dBA_string[0]),
    minimum_dBA: parseFloat(dBA_string[1]),
    average_dBA: parseFloat(dBA_string[2])
  };
}
