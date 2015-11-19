"use strict";angular.module("lelylan-lab.directives.project",["lelylan-lab.directives.project.directive","lelylan-lab.client.project","lelylan-lab.client.utils","ngTouch","ngAnimate"]),angular.module("lelylan-lab.directives.project").provider("lelylanLabClientConfig",function(){var a;return a={endpoint:"http://lelylan-lab.herokuapp.com"},{configure:function(b){return angular.extend(a,b)},$get:function(){return a}}});var client=angular.module("lelylan-lab.client.utils",[]);client.factory("LelylanLabClientUtils",[function(){var a={};return a.headers=function(){return{}},a.merge=function(a,b){for(var c in b)a[c]=b[c];return a},a}]);var client=angular.module("lelylan-lab.client.project",[]);client.factory("Project",["$http","lelylanLabClientConfig","LelylanLabClientUtils",function(a,b,c){var d={},e=b.endpoint+"/projects";return d.find=function(b,d){var f={};return a.get(e+"/"+b,c.merge(f,d))},d.public=function(b,d){var f={params:b};return a.get(e+"/public",c.merge(f,d))},d}]),angular.module("lelylan-lab.directives.project.directive",[]),angular.module("lelylan-lab.directives.project.directive").directive("lyProject",["$rootScope","$timeout","$http","Project",function(a,b,c,d){var e={restrict:"EA",replace:!0,template:'<div class="ly-project ly-project-item-card"><div class="ly-project-item-card ly-project-list-item"><a href="{{project.link}}" target="_blank"><img class="ly-project-user-avatar" src="{{project.image}}"/></a><h1 class="ly-project-title"><a href="{{project.link}}" target="_blank">{{project.name}}</a></h1><p class="ly-project-tags"><a class="ly-project-tag" ng-repeat="tag in project.tags" href="http://localhost:9000#projects/{{tag}}" target="_blank">{{tag}}</a></p><p class="ly-project-description">{{project.description}}</p><p class="ly-project-link"><a href="{{project.link}}" target="_blank">Learn more about this project &rarr;</a></p><p class="ly-project-link ly-project-footer" ng-click="embedded=true" ng-show="!embedded"><a class="ly-project-embed"><span>&rarrlp;</span> embed</a> <a href="http://lelylan.com" target="_blank">With &hearts; lelylan.com</a></p><p class="ly-project-embed-input ly-project-link ly-project-footer" ng-show="embedded"><input type="text" ng-model="embed"></input></p></div></div>',scope:{lyProjectId:"@"}};return e.link=function(a){a.view={path:"/loading"},a.$watch("lyProjectId",function(b){b&&d.find(b).success(function(b){a.project=b,a.view.path="/default",a.embed='<div ng-app="project-app"><ly-project ly-project-id="'+a.project.id+'"></ly-project><link rel="stylesheet" href="http://lelylan.github.io/lab-directives-ng/styles/main.css"><script src="http://lelylan.github.io/lab-directives-ng/scripts/vendor.js"></script><script src="http://lelylan.github.io/lab-directives-ng/scripts/scripts.js"></script><script>angular.module(\'project-app\', [\'lelylan-lab.directives.project\']);</script></div>'}).error(function(){a.view.path="/message",a.message={title:"Project not found",description:"Most probably the project you are lloking at was deleted. Find more at lelylan.com"}})})},e}]);