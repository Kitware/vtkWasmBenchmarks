module.exports = function (app) {
  '/': {
    target: 'http://localhost:3001',
    bypass: function (req, res, opts) {
      res.setHeader("Cross-Origin-Opener-Policy", "same-origin");
      res.setHeader("Cross-Origin-Embedder-Policy", "require-corp");
    },
  },
};
