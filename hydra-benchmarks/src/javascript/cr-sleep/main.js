const timer = ms => new Promise( res => setTimeout(res, ms));

async function main(event) {
  var sleep =  event.sleep;
  return timer(sleep*1000);
};
