const webpack = require('webpack')
const MomentTimezoneDataPlugin = require('moment-timezone-data-webpack-plugin')
const CleanWebpackPlugin = require('clean-webpack-plugin')
const merge = require('webpack-merge')
const HtmlWebpackPlugin = require('html-webpack-plugin')
// const FaviconsWebpackPlugin = require('favicons-webpack-plugin');
const autoprefixer = require('autoprefixer')
const MiniCssExtractPlugin = require('mini-css-extract-plugin');
const eslintFormatter = require('react-dev-utils/eslintFormatter')
const common = require('./common.js')
const util = require('./util')

const resolve = util.resolve

const prodEnv = {
  NODE_ENV: JSON.stringify('production'),
  PLATFORM_ENV: JSON.stringify('web'),
  SERVER_URL: JSON.stringify('https://api.cyberrepublic.org'),
  FORUM_URL: JSON.stringify('https://forum.cyberrepublic.org'),
  SSO_URL: JSON.stringify('https://forum.cyberrepublic.org/session/sso_login'),
  CR_VERSION: JSON.stringify(process.env.CR_VERSION),
  GA_ID: JSON.stringify(process.env.GA_ID),
  GOOGLE_MAPS_API_KEY: JSON.stringify(process.env.GOOGLE_MAPS_API_KEY),
}

const stagingEnv = {
  NODE_ENV: JSON.stringify('production'),
  PLATFORM_ENV: JSON.stringify('web'),
  SERVER_URL: JSON.stringify('https://staging-api.cyberrepublic.org'),
  FORUM_URL: JSON.stringify('http://18.136.60.61:3100'),
  SSO_URL: JSON.stringify('http://18.136.60.61:3100/session/sso_login'),
  CR_VERSION: JSON.stringify(process.env.CR_VERSION),
}

const devEnv = {
  NODE_ENV: JSON.stringify('development'),
  PLATFORM_ENV: JSON.stringify('web'),
  SERVER_URL: JSON.stringify('http://local.ebp.com:3000'),
  FORUM_URL: JSON.stringify('http://local.ebp.com:3100'),
  SSO_URL: JSON.stringify('http://local.ebp.com:3100/session/sso_login'),
  CR_VERSION: JSON.stringify(process.env.CR_VERSION),
}


module.exports = merge(common, {
  cache: false,
  performance: {
    hints: false,
  },
  output: {
    path: resolve('dist'),
    chunkFilename: 'static/js/[name].[contenthash:8].bundle.js',
    filename: 'static/js/[name].[contenthash:8].js',
    publicPath: '/',
  },
  // devtool: 'hidden-source-map',
  stats: {
    //need it
    entrypoints: false,
    children: false,
  },
  module: {
    strictExportPresence: true, //need this
    rules: [
      // {
      //     test: /\.(js|jsx)$/,
      //     enforce: 'pre',
      //     use: [
      //         {
      //             options: {
      //                 formatter: eslintFormatter,
      //                 eslintPath: require.resolve('eslint'),
      //             },
      //             loader: require.resolve('eslint-loader'),
      //         },
      //     ],
      //     include: resolve('src'),
      // },
      {
        oneOf: [
          {
            test: /\.svg$/,
            exclude: /node_modules/,
            include: resolve('src'),
            use: [
              {
                loader: '@svgr/webpack',
              },
              {
                loader: 'file-loader',
                options: {
                  limit: 10000,
                  name: '[name].[hash:8].[ext]',
                  publicPath: '/static/media',
                  outputPath: 'static/media',
                },
              },
            ],
          },
          {
            test: /\.(png|jpg|gif)$/,
            include: resolve('src'),
            loader: 'file-loader',
            options: {
              limit: 10000,
              name: '[name].[hash:8].[ext]',
              publicPath: '/static/media',
              outputPath: 'static/media',
            },
          },
          {
            test: /\.(js|jsx)$/,
            include: resolve('src'),
            loader: require.resolve('babel-loader'),
            exclude: /node_modules/,
            options: {
              plugins: ['react-html-attrs'],
              compact: true,
            },
          },
          {
            test: /\.css$/,
            use: [MiniCssExtractPlugin.loader, { loader: 'css-loader' }, { loader: 'postcss-loader' }],
          },
          {
            test: /\.scss$/,
            include: resolve('src'),
            exclude: [/jest/, /node_modules/, /mobile\.scss$/],
            use: [
              MiniCssExtractPlugin.loader,
              {
                loader: require.resolve('css-loader'),
                options: {
                  importLoaders: 1,
                  minimize: true,
                  sourceMap: true,
                  publicPath: resolve('dist'),
                },
              },
              {
                loader: require.resolve('postcss-loader'),
                // options: {
                //     ident: 'postcss',
                //     plugins: () => [
                //         require('postcss-flexbugs-fixes'),
                //         autoprefixer({
                //             browsers: [
                //                 '>1%',
                //                 'last 4 versions',
                //                 'Firefox ESR',
                //                 'not ie < 9', // React doesn't support IE8 anyway
                //             ],
                //             flexbox: 'no-2009',
                //         }),
                //     ],
                // },
              },
              {
                loader: require.resolve('sass-loader'),
              },
            ]
          },
          {
            test: /mobile\.scss$/,
            include: resolve('src'),
            exclude: [/jest/, /node_modules/],
            use: [
              MiniCssExtractPlugin.loader,
              {
                loader: require.resolve('css-loader'),
                options: {
                  importLoaders: 1,
                  minimize: true,
                  sourceMap: true,
                  publicPath: resolve('dist'),
                },
              },
              {
                loader: require.resolve('postcss-loader'),
              },
              {
                loader: require.resolve('sass-loader'),
              },
            ]
          },
          {
            test: /\.less$/,
            use: [MiniCssExtractPlugin.loader, 'css-loader', 'postcss-loader', {
              loader: 'less-loader',
              options: {
                javascriptEnabled: true
              }
            }],
          },
          {
            test: /\.(eot|ttf|woff|woff2)$/,
            exclude: /node_modules/,
            loader: 'file-loader',
            options: {
              name: '[name].[ext]',
              publicPath: '/static/css/fonts',
              outputPath: 'static/css/fonts',
            },
          },
          {
            loader: require.resolve('file-loader'),
            exclude: [/\.js$/, /\.html$/, /\.json$/],
            options: {
              name: 'static/media/[name].[hash:8].[ext]',
            },
          },
        ],
      },
    ],
  },
  plugins: [
    new CleanWebpackPlugin(resolve('dist')),
    // new FaviconsWebpackPlugin({
    //     logo: path.join(__dirname, 'public/favicon.png'),
    //     prefix: 'static/media/icon[hash:8]/',
    //     icons: { favicons: true },
    // }),
    new HtmlWebpackPlugin({
      sinject: true,
      template: resolve('public/index.html'),
      minify: {
        removeComments: true,
        collapseWhitespace: true,
        removeRedundantAttributes: true,
        useShortDoctype: true,
        removeEmptyAttributes: true,
        removeStyleLinkTypeAttributes: true,
        keepClosingSlash: true,
        minifyJS: true,
        minifyCSS: true,
        minifyURLs: true,
      },
      chunksSortMode: 'none'
    }),
    new MiniCssExtractPlugin({
      filename: 'static/css/[name].[contenthash:8].css',
      chunkFilename: 'static/css/[name].[contenthash:8].chunk.css'
    }),
    new webpack.DefinePlugin({
      'process.env':
        process.env.NODE_ENV === 'production'
          ? prodEnv
          : process.env.NODE_ENV === 'staging'
            ? stagingEnv
            : devEnv,
    }),
    new webpack.optimize.AggressiveMergingPlugin(),
    new webpack.NoEmitOnErrorsPlugin(),
    new webpack.IgnorePlugin(/^\.\/locale$/, /moment$/),
    MomentTimezoneDataPlugin({ startYear: 2018, endYear: 2100 }),
  ],
})
