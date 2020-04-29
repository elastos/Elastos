const path = require('path')
const HtmlWebpackPlugin = require('html-webpack-plugin')
const MiniCssExtractPlugin = require('mini-css-extract-plugin')
const webpack = require('webpack')
const webpackNotifier = require('webpack-notifier')
const merge = require('webpack-merge')
const common = require('./common.js')
const util = require('./util')

const resolve = util.resolve

module.exports = merge(common, {
  cache: true, // for rebuilding faster
  output: {
    path: resolve('dev_dist'),
    filename: 'static/js/bundle.js',
    publicPath: '/',
    pathinfo: true,
  },
  module: {
    rules: [
      {
        test: /\.css$/,
        use: ['style-loader', 'css-loader', 'postcss-loader'],
      },
      {
        test: /\.scss$/,
        exclude: /node_modules/,
        use: ['style-loader', 'css-loader', 'postcss-loader', 'sass-loader'],
      },
      {
        test: /\.less$/,
        use: ['style-loader', 'css-loader', 'postcss-loader', {
          loader: 'less-loader',
          options: {
            javascriptEnabled: true
          }
        }],
      },
      {
        test: /\.svg$/,
        exclude: /node_modules/,
        use: ['@svgr/webpack', 'url-loader'],
      },
      {
        test: /\.(eot|ttf|woff|woff2)$/,
        exclude: /node_modules/,
        loader: 'file-loader',
      },
      {
        test: /\.js$/,
        exclude: /node_modules/,
        use: {
          loader: 'babel-loader',
          options: {
            presets: ['es2015', 'react', 'stage-0'],
            cacheDirectory: true,
            plugins: ['react-hot-loader/babel', 'react-html-attrs'],
          },
        },
      },
      {
        test: /\.svg$/,
        exclude: /node_modules/,
        use: ['svg-inline-loader'],
      },
      {
        test: /\.(png|jpg)$/,
        exclude: /node_modules/,
        use: [
          {
            loader: 'url-loader',
            options: { name: 'image/[name]-[hash:8].[ext]' },
          },
        ],
      },
    ],
  },
  devtool: 'cheap-module-source-map',
  devServer: {
    historyApiFallback: true,
    contentBase: 'dev_dist',
    port: 3001,
    hot: true,
    host: '0.0.0.0',
    /*
        headers: {
            'X-Frame-Options': 'allow-from https://www.facebook.com/'
        }, */
    watchOptions: {
      ignored: /node_modules/,
      aggregateTimeout: 2000,
      poll: 5000,
    },
    proxy: {
      '/api': 'http://localhost:3000',
    },
    compress: true,
    disableHostCheck: true,
  },
  plugins: [
    new webpackNotifier(),
    new HtmlWebpackPlugin({
      inject: true,
      template: resolve('public/index.html'),
      minify: {
        minifyJS: false,
        minifyCSS: false,
      },
      chunksSortMode: 'none'
    }),
    new webpack.DefinePlugin({
      'process.env': {
        NODE_ENV: JSON.stringify('development'),
        PLATFORM_ENV: JSON.stringify('web'),
        SERVER_URL: JSON.stringify('http://localhost:3001'),
        FORUM_URL: JSON.stringify('http://localhost:3100'),
        SSO_URL: JSON.stringify('http://localhost:3100/session/sso_login'),
        CR_VERSION: JSON.stringify(process.env.CR_VERSION),
      },
    }),
    new webpack.optimize.OccurrenceOrderPlugin(),
    new webpack.HotModuleReplacementPlugin(),
    new webpack.NoEmitOnErrorsPlugin(),
    new webpack.IgnorePlugin(/^\.\/locale$/, /moment$/),
  ],
})
