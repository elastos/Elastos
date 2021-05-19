'use strict';
const sass = require('node-sass');

module.exports = function(grunt) {

  require('load-grunt-tasks')(grunt);

  // Project Configuration
  grunt.initConfig({
    pkg: grunt.file.readJSON('package.json'),
    exec: {},
    watch: {
      options: {
        dateFormat: function (time) {
          grunt.log.writeln('The watch finished in ' + time + 'ms at ' + (new Date()).toString());
          grunt.log.writeln('Waiting for more changes...');
        },
      },
      sass: {
        files: ['sass/*.scss', 'sass/**/*.scss'],
        tasks: ['concat', 'sass']
      }
    },
    concat: {
      dist: {
        src: ['sass/colors.scss', 'sass/*.scss', 'sass/**/*.scss'],
        dest: 'static/css/custom.scss'
      }
    },
    sass: {
      dist: {
        options: {
          implementation: sass,
          style: 'compact',
          sourcemap: 'true'
        },
        files: {
          'static/css/custom.css': 'static/css/custom.scss'
        }
      }
    }
  })

  grunt.registerTask('default', ['concat', 'sass']);
}
