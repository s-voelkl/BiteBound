// Give VS Code IntelliSense for uibuilder
// <reference path="../types/uibuilder.d.ts"/>

/** The simplest use of uibuilder client library
 * See the docs if the client doesn't start on its own.
 */

// Listen for incoming messages from Node-RED and action
uibuilder.onChange('msg', (msg) => {
    console.log(msg);
    // do stuff with the incoming msg
})

$('#btn2').addEventListener('click', () => {
    uibuilder.send({ topic: 'button', payload: 'clicked' })
})