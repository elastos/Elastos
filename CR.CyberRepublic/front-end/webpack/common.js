const path = require('path');
const UglifyJsPlugin = require('uglifyjs-webpack-plugin');
const OptimizeCSSAssetsPlugin = require('optimize-css-assets-webpack-plugin');
const CopyWebpackPlugin = require('copy-webpack-plugin');
const util = require('./util');
const resolve = util.resolve;

module.exports = {
    entry: ['babel-polyfill', resolve('src/app.js')],
    optimization: {
        minimizer: [
            new UglifyJsPlugin({
                cache: true,
                parallel: true,
                sourceMap: true, // set true is you want JS source map
                uglifyOptions: {
                    ecma: 6,
                    compress: {
                        drop_console: process.env.NODE_ENV === 'production',
                    },
                },
            }),
            new OptimizeCSSAssetsPlugin(),
        ],
        splitChunks: {
            chunks: 'async',
            minSize: 30000,
            minChunks: 1,
            maxAsyncRequests: 5,
            maxInitialRequests: 3,
            automaticNameDelimiter: '~',
            name: true,
            cacheGroups: {
                vendors: {
                    test: /[\\/]node_modules[\\/]/,
                    priority: -10,
                },
                styles: {
                    name: 'styles',
                    test: /\.(scss|css)$/,
                    chunks: 'all',
                    minChunks: 1,
                    reuseExistingChunk: true,
                    enforce: true,
                },
                default: {
                    minChunks: 2,
                    priority: -20,
                    reuseExistingChunk: true,
                },
            },
        },
    },
    resolve: {
        extensions: ['.js', '.json', '.css', '.less', '.scss', '.sass', '.jsx'],
        alias: {
            '@': resolve('src'),
            'img': resolve('src/img')
        }
    },
    plugins : [
        new CopyWebpackPlugin([
            {
                from: 'src/assets',
                to: 'assets'
            }
        ])
    ]
};
