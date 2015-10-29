// We use the fake 'PBL' symbol as default
var symbol = 'PBL';

// Fetch stock data for a given stock symbol (NYSE or NASDAQ only) from markitondemand.com
// & send the stock price back to the watch via app message
// API documentation at http://dev.markitondemand.com/#doc
function fetchStockQuote(symbol, isInitMsg) {
  var req = new XMLHttpRequest();
  
  // build the GET request
  req.open('GET', 'http://dev.markitondemand.com/Api/Quote/json?symbol=' + symbol, true);
  req.onload = function(e) {
    if (req.readyState == 4) {
      // 200 - HTTP OK
      if(req.status == 200) {
        console.log(req.responseText);
        
        var response = JSON.parse(req.responseText);
        if (response['Message']) {
          // the markitondemand API sends a response with a Message
          // field when the symbol is not found
          Pebble.sendAppMessage({ 'QuoteKeyPrice': 'Not Found' });
        }
        if (response['Data']) {
          // data found, look for LastPrice
          var price = response['Data']['LastPrice'];
          console.log('Price is ' + price);

          Pebble.sendAppMessage({
            'QuoteKeyInit': true,
            'QuoteKeySymbol': symbol,
            'QuoteKeyPrice': '$' + price.toString()
          }, function(e) {
            console.log('sent');
          }, function() {
            console.log('failed');
          });
        }
      } else {
        console.log('Request returned error code ' + req.status.toString());
      }
    }
  };
  req.send(null);
}

// Set callback for the app ready event
Pebble.addEventListener('ready', function(e) {
  console.log('connect!');
  console.log(e.type);
  // Fetch saved symbol from local storage (using
  // standard localStorage webAPI)
  symbol = localStorage.getItem('symbol');
  if (!symbol) {
    symbol = 'PBL';
  }
  fetchStockQuote(symbol, true);
});

// Set callback for appmessage events
Pebble.addEventListener('appmessage', function(e) {
  console.log('message');
  if (e.payload['QuoteKeyInit']) {
    fetchStockQuote(symbol, true);
  } else if (e.payload['QuoteKeyFetch']) {
    fetchStockQuote(symbol, false);
  } else if (e.payload['QuoteKeySymbol']) {
    symbol = e.payload['QuoteKeySymbol'];
    localStorage.setItem('symbol', symbol);
    fetchStockQuote(symbol, false);
  }
});

