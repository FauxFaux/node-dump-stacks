const addon = require('node-gyp-build')(__dirname);
module.exports = {
  native: addon,
};
