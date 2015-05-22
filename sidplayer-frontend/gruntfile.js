module.exports = function(grunt) {
    'use strict';
    require('load-grunt-tasks')(grunt);

    grunt.initConfig({
        browserify: {
            dev: {
                options: {
                    watch: true,
                    browserifyOptions: {
                        debug: true
                    }
                },
                src: 'app/app.js',
                dest: 'build/app.js'
            },
            build: {
                options: {
                    watch: false
                },
                src: 'app/app.js',
                dest: 'dist/app.tmp.js'
            }
        },
        uglify: {
            build: {
                files: {
                    'dist/app.js': ['dist/app.tmp.js']
                }
            }
        },
        clean: {
            build: ['dist/app.tmp.js']
        },
        sass: {
            dev: {
                options: {
                    sourceMap: true,
                    precision: 10
                },
                files: {
                    'build/styles.css': 'app/styles.scss'
                }
            },
            build: {
                options: {
                    sourceMap: false,
                    outputStyle: 'compressed',
                    precision: 10
                },
                files: {
                    'dist/styles.css':'app/styles.scss'
                }
            }
        },
        autoprefixer: {
            options: {
                browsers: [
                    'last 2 versions',
                    'ie 8',
                    'ie 9'
                ]
            },
            dev: {
                src: 'build/styles.css'
            },
            build: {
                src: 'dist/styles.css'
            }
        },
        connect: {
            server: {
                options: {
                    port: 1337,
                    hostname: '*',
                    base: 'build',
                    livereload: true
                }
            }
        },
        copy: {
            dev: {
                files: [
                    {
                        expand: true,
                        cwd: 'app',
                        src: ['index.html'],
                        dest: 'build/',
                        filter: 'isFile'
                    },
                    {
                        expand: true,
                        cwd: 'app',
                        src: ['static/**/**'],
                        dest: 'build/',
                        filter: 'isFile'
                    }
                ]
            },
            build: {
                files: [
                    {
                        expand: true,
                        cwd: 'app',
                        src: ['index.html'],
                        dest: 'dist/',
                        filter: 'isFile'
                    },
                    {
                        expand: true,
                        cwd: 'app',
                        src: ['static/**/**'],
                        dest: 'dist/',
                        filter: 'isFile'
                    }
                ]
            }
        },
        watch: {
            build: {
                files: ['build/*.*'],
                options: {
                    livereload: true
                }
            },
            statics: {
                files: ['app/index.html'],
                tasks: ['copy:dev']
            },
            scss: {
                files: ['app/**/*.scss'],
                tasks: ['sass:dev', 'autoprefixer:dev']
            }
        }
    });

    grunt.registerTask('dev', ['browserify:dev', 'copy:dev', 'sass:dev', 'autoprefixer:dev', 'connect:server', 'watch']);

    /**
     * Register the task like this so we can use the timing
     * for this task only
     */
    grunt.registerTask('build', 'Build a release of the app', function() {
        require('time-grunt')(grunt);
        var tasks = ['browserify:build', 'uglify:build', 'copy:build', 'sass:build', 'autoprefixer:build', 'clean:build'];
        grunt.task.run(tasks);
    });
};
