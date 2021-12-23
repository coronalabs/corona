const gulp = require('gulp');
const htmlreplace = require('gulp-html-replace');
const webserver = require('gulp-webserver');
const rename = require("gulp-rename");
const zip = require('gulp-zip');
const request = require('request');
const fs = require('fs');
const open = require('open');
const path = require('path');
const uuid = require('uuid/v1');

const config = require('./config.json');
const SDK_PATH = 'https://connect.facebook.net/en_US/fbinstant.6.0.js'
const BUILD_FOLDER = './build'
const LIB_FILES = [
    './node_modules/html2canvas/dist/html2canvas.min.js'
]

function make() {
    var sourceFiles = [
        'js/**/*',
        'css/**/*',
        'img/**/*',
        '!js/mock/**/*',
        '!css/mock/**/*',
        '!assets/mock/**/*'
    ];
    var sdkPath = SDK_PATH

    return Promise.all([
        new Promise(function(resolve, reject){
            gulp.src(LIB_FILES)
            .on('error', reject)
            .pipe(gulp.dest('./js/lib/'))
            .on('end', resolve)
        }),
        new Promise(function(resolve, reject){
            gulp.src(sourceFiles, { base: './' })
            .on('error', reject)
            .pipe(gulp.dest(BUILD_FOLDER))
            .on('end', resolve)
        }),
        new Promise(function(resolve, reject){
            gulp.src('./index.html')
            .on('error', reject)
            .pipe(htmlreplace({
                'js': sdkPath
            }))
            .pipe(gulp.dest(BUILD_FOLDER))
            .on('end', resolve)
        }),
    ]);
}

function archive(archivesFolder, filename){
    return new Promise(function(resolve, reject){
        console.log('Going to create zip archive: ' + archivesFolder + '/' + filename);
        gulp.src([
            __dirname + '/build/**',
            '!'+ __dirname + '/build/archives/**',
            '!**.zip'
        ])
        .pipe(zip(filename))
        .on('error', reject)
        .pipe(gulp.dest(archivesFolder))
        .on('end', function(){
            console.log('ZIP archive created')
            resolve();
        })
    })
}

function upload(archivesFolder, filename) {
    return new Promise(function(resolve, reject){
        console.log('Going to upload archive: ' + archivesFolder + '/' + filename);
        request.post({
            url: 'https://graph-video.facebook.com/'+config.FB_appId+'/assets',
            formData: {
                'access_token' : config.FB_uploadAccessToken,
                'type': 'BUNDLE',
                'comment': 'Uploaded via gulp task',
                'asset': {
                    value: fs.createReadStream(__dirname + '/'+ archivesFolder + '/' + filename),
                    options: {
                        filename: filename,
                        contentType: 'application/octet-stream'
                    }
                }
            },
        }, function(error, response, body) {
            if (error || !body) reject(error);
                try {
                    var body = JSON.parse(response.body);
                    if (body.success) {
                        console.log('Bundle uploaded via the graph API');
                        console.log('Don\'t forget you need to publish the build');
                        console.log('Opening developer dashboard...');
                        open('https://developers.facebook.com/apps/'+ config.FB_appId +'/hosting/')
                        resolve();
                    } else {
                        reject('Upload failed. Unexpected Graph API response: ' + response.body);
                    }            
                } catch (e) {
                    reject('Upload failed. Invalid response response: ' + response.body);
                }
        });
    });
}

/*
 * `gulp mock` 
 *  Runs the game on http://localhost:8000/ using the Mock SDK
 *  It won't communicate with Facebook's services, but it speeds up local development.
 *  Use it for rapid prototyping and for developing game features that are not connected to the SDK.
 * 
 */
 gulp.task('mock', function() {
    gulp.src(LIB_FILES)
        .pipe(gulp.dest('./js/lib/'));
    gulp.src('./')
      .pipe(webserver({
          open: true,
          port: 8000,
          liveReload:true
      }));
});

/*
 * `gulp test`
 * Runs an SSL webserver with the game on https://localhost:8080
 * Opens the browser in the embedded player (https://developers.facebook.com/docs/games/instant-games/test-publish-share)
 * and uses the production SDK. This means all FBInstant functions will be 
 * actually executed by Facebook's services.
 * 
 */
 gulp.task('test', function() {
  make()
    .then(function() {
        gulp.src(BUILD_FOLDER)
          .pipe(webserver({
              https: true,
              port: 8000,
              open: 'https://www.facebook.com/embed/instantgames/'+config.FB_appId+'/player?game_url=https://localhost:8000'
          }));      
    })
    .catch(function(error){
        console.log('gulp:test failed', error);
    })

});

/*
 * `gulp push`
 * Compresses the game into a .zip archive and uploads it to the Developer website
 *
 */ 
 gulp.task('push', function() {
    const filename = uuid() + '.zip';
    const archivesFolder = 'build/archives';
    
    make()
        .then(function(){
            archive(archivesFolder, filename)
            .then(function(){
                upload(archivesFolder, filename)
                .then(function(){
                    console.log('Success!');
                })
            })
        })

});