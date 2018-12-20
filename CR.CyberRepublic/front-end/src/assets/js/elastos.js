var mobile = false;
var mobileBreak = 768;
if($(window).width() <= mobileBreak){mobile = true;}
var is_firefox = navigator.userAgent.toLowerCase().indexOf('firefox') > -1;
var path;
function freezePage(){$('body').css({'width':'100%','height':'100%','overflow':'hidden'});}
function unfreezePage(){$('body').css({'width':'','height':'','overflow':''});}
function animScroll(sec, speed, offset){
	activeOffset = $(sec).offset().top+offset;
	TweenMax.to('html,body', speed, {scrollTop:activeOffset, ease:Expo.easeInOut});
}
function randomOffsets(elem, limit){
	animD = (Math.random() * (limit - 0)).toFixed(2);
	elem.css({'animation-delay':-animD+'s'});
}





//! - GLOBAL: WINDOW RESIZE

var winW;
var winH;
$(window).resize(function(){
	winW = $(window).width();
	winH = window.innerHeight;
	//console.log(winW+' / '+winH);

	// update sticky
	updateSticky();

	// update scaler elements
	updateScales();

	// update model swiper
	if(winW<=650){
		if(!draggableOn){
			updateModelW();
			createModelSwiper();
		}
	} else {
		if(draggableOn){
			removeModelSwiper();
		}

		// update slideshow
		updateModelW();
	}

	// update team grid
	updateTeam();
})
$(window).resize();







//! - GLOBAL: STICKY ELEMENTS

var stickyOpen = false;
var stickyEmail = false;
var topDif = 0;
var stickyBreak = 1024;
var stickyH = 77;
var lastSt = 0;
var autoOff = false;

function setSticky(){

	// drop sticky bar on scroll up
	if(sT<lastSt && winW>=stickyBreak){
		if(!stickyOpen && sT > 110){
			$('.sticky').addClass('open');
			TweenMax.to($('.sticky'), .75, {delay:.2, 'transform':'translate3d(0px, '+ 0 +'px, 0px)', 'display':'block', ease:Power3.easeOut});
			stickyOpen = true;
		}

		// remove sticky if top of page
		if(stickyOpen && sT <= 100){
			$('.sticky').removeClass('open');
			//TweenMax.to($('.sticky'), .75, {'transform':'translate3d(0px, '+ -stickyH +'px, 0px)', ease:Power3.easeOut});
			stickyOpen = false;
		}
	}

	// remove sticky on scroll down
	if(sT>lastSt){
		if(stickyOpen){
			$('.sticky').removeClass('open');
			TweenMax.to($('.sticky'), .75, {'transform':'translate3d(0px, '+ -stickyH +'px, 0px)', ease:Power3.easeInOut});
			stickyOpen = false;
		}
	}

	if(sT<=0 && !autoOff){
		TweenMax.killTweensOf($('.sticky'));
		autoOff = true;
		TweenMax.to($('.sticky'), .75, {'transform':'translate3d(0px, '+ -stickyH +'px, 0px)', 'display':'none', ease:Power3.easeOut, onComplete:function(){
			autoOff = false;
		}});

		stickyOpen = false;
	}


	// set menu button sticky
	if(sT>20 && winW>900){
		$('#globalMenu').addClass('on');
	}
	if(sT<=20){
		$('#globalMenu').removeClass('on');
	}

	lastSt = sT;
}

function updateSticky(){
// 	if(winW<=900 && $('#globalMenu').hasClass('on')){
// 		$('#globalMenu').removeClass('on');
// 	} else if(!$('#globalMenu').hasClass('on')){
// 		$('#globalMenu').addClass('on');
// 	}

	if(winW<=stickyBreak && $('.sticky').hasClass('open')){
		$('.sticky').removeClass('open');
		TweenMax.to($('.sticky'), .75, {'transform':'translate3d(0px, '+ -stickyH +'px, 0px)', ease:Power3.easeInOut});
		stickyOpen = false;
	}
}





//! - GLOBAL: CONTACT FORM

var formSent = false;
var formURL;

$('.signup-form').submit(function(){
	if(validateForm($(this))){
		sendForm($(this));
	}
	return false;
});

function sendForm(formObj){

	formURL = formObj.attr('action');

	// animation actions
	formObj.addClass('sending');

	var formData = formObj.serialize();

	$.ajax({
		url: formURL,
		type: 'GET',
		data: formData,
		dataType: "jsonp",
		jsonp: "c",
		contentType: "application/json; charset=utf-8",

		success: function(data){
			formSent = true;
			console.log(data.result);
			setTimeout(function(){
				formObj.removeClass('sending').addClass('thanks');
				TweenMax.to(formObj.find('input[name="EMAIL"]'), .3, {opacity:0, onCompleteParams:[formObj.find('input[name="EMAIL"]')], onComplete:function(t){
					t.val('We will be in contact shortly!');
					TweenMax.to(t, .3, {opacity:1})
					TweenMax.to(t, .3, {delay:2, opacity:0})
					TweenMax.to(t, .3, {delay:2.3, opacity:1, onStart:function(){
						t.val('');
						formObj.removeClass('thanks');
					}})
				}})
				//TweenMax.to('.thanks', .5, {'opacity':1, 'display':'block'})
			}, 1500);
		}
	});

}

function validateForm(formObj){
	var vNum = 0;
	$(formObj).find('[data-type="req"]').each(function(){
		if($(this).val() == ""){
			vNum++;
		}
	});
	if(vNum==0){
		return true;
	} else {
		formObj.find('.email-wrap').addClass('error');
		return false;
	}
}

$('input').focus(function(){
	$(this).parents('.email-wrap').removeClass('error');
	formSent = false;
})






//! - GLOBAL: LOADER

// skip loader
// $('#loader').hide();
// $(window).on('load', function(){
// 	$(window).resize();
// })

var loadReady = false;
var loadSp = .7;
var loadDel = .25;
var topDel = loadDel*6;
//var endDel = ((topDel*2)*1000) - 200;
var endDel = 2;
var loadEase = 'Power3.easeInOut';

freezePage();

function animateLogo(){
	TweenMax.to($('.logo-mark>img'), .5, {scaleX:1, scaleY:1, opacity:1, ease:Power3.easeOut});

	// set for listener to open page
	setTimeout(function(){
		loadReady = true;
	}, 100)

}

setTimeout(function(){animateLogo()}, 500);


var loadChecker = setInterval(function(){

	if(loadReady){

		// slide over
		TweenMax.to('.load-clip', .75, {delay:.5, x:0, ease:Power3.easeInOut})
		TweenMax.to('.logo-text', .5, {delay:.7, 'width':'259px', opacity:1, ease:Power3.easeInOut})

		// reveal page
		loadSC = .6; if(winW<=650){loadSC = .4;}
		TweenMax.to('.load-clip', .5, {delay:1.5, opacity:0, scaleX:loadSC, scaleY:loadSC, ease:Power3.easeInOut})
		TweenMax.to('#loader', .75, {delay:1.7, opacity:0, 'display':'none', onComplete:function(){
			unfreezePage();
		}});
		$(window).resize();
		clearInterval(loadChecker);
	}
}, 30)






//! - GLOBAL: VIDEO OVERLAY

lb_video = document.getElementById("lbvid");

function openVideoOverlay(){
	vid = 'Elastos';
	freezePage();

	$('#video-overlay').css({'display':'block'});
	TweenMax.to($('#video-overlay'), .5, {opacity:1});

	lb_video.pause();

	vidHTML = '<source src="/assets/video/' + vid + '.webm" type="video/webm">';
	vidHTML += '<source src="/assets/video/' + vid + '.ogv" type="video/ogg">';
	vidHTML += '<source src="/assets/video/' + vid + '.mp4" type="video/mp4">';

	$('#lbvid').html(vidHTML);
	$('#lbvid').attr('poster',vid+'.jpg');
	lb_video.load();

	lb_video.addEventListener('loadedmetadata', function() {
		lb_video.play();
	}, false);
}

function closeVideoOverlay(){
	unfreezePage();
	TweenMax.to($('#video-overlay'), .5, {opacity:0, 'display':'none'});
	lb_video.pause();
}

$('#video-overlay').click(function(){
	closeVideoOverlay();
})

$('#lbvid').click(function(e){
	e.stopPropagation();
})

$('.video-btn').click(function(){
	openVideoOverlay();
	return false;
})





//! SPECIAL PARTS ADDED AFTER LOAD

$('.cta-btn').prepend('<div class="cta-border tl"></div><div class="cta-border tr"></div><div class="cta-border left"></div><div class="cta-border right"></div><div class="cta-border bot"></div><div class="cta-border-cover bl"></div><div class="cta-border-cover br"></div>');

$('.roadmap-box').each(function(i){
	$(this).attr('data-num',(i+1));
})
$('.roadmap-row').each(function(i){
	$(this).attr('data-num',(i+1));
})

$('.social').find('li:not(.title)').prepend('<div class="cta-border tl"></div><div class="cta-border tr"></div><div class="cta-border left"></div><div class="cta-border right"></div><div class="cta-border bot"></div>');

$('.social, .resources').find('a').attr('target','_blank');





//! UPDATE SCALES

var baseW = 764;

function updateScales(){
	curSC = ($('#what').find('.scale-wrap').width()/baseW).toFixed(3);
	//console.log(curSC)
	$('#what').find('.background').css({'transform':'scale('+curSC+')'})
}





//! - SECTION: 3 PILLARS

var tl1 = new TimelineMax({repeat:-1, onRepeat: fireRadio1});
var motionPath1;

if ($('#pillar-path1').length) {
	motionPath1 = MorphSVGPlugin.pathDataToBezier("#pillar-path1");
}

tl1.set('.dot.path-move[data-num="1"]', {xPercent:-50, yPercent:-50});
tl1.to('.dot.path-move[data-num="1"]', 4, {bezier: {values:motionPath1, type:"cubic"}, ease:Linear.easeNone});
tl1.add(resetRadio1, tl1.duration()/2);
tl1.add(fireRadio1a, tl1.duration()/3);
tl1.pause();

function fireRadio1(){
	$('.pillar-box[data-num="4"]').find('.radio-group[data-num="1"]').find('.radio-circle').addClass('radio-ping');
	$('.pillar-box[data-num="4"]').find('.radio-group[data-num="3"]').find('.radio-circle').removeClass('radio-ping');
}
function resetRadio1(){$('.pillar-box[data-num="4"]').find('.radio-group[data-num="1"]').find('.radio-circle').removeClass('radio-ping');}
function fireRadio1a(){$('.pillar-box[data-num="4"]').find('.radio-group[data-num="3"]').find('.radio-circle').addClass('radio-ping');}

var tl2 = new TimelineMax({repeat:-1, onRepeat: fireRadio2});
var motionPath2;

if ($('#pillar-path2').length) {
	motionPath2 = MorphSVGPlugin.pathDataToBezier("#pillar-path2");
}

tl2.set('.dot.path-move[data-num="2"]', {xPercent:-50, yPercent:-50});
tl2.to('.dot.path-move[data-num="2"]', 4, {bezier: {values:motionPath2, type:"cubic"}, ease:Linear.easeNone});
tl2.add(resetRadio2, tl2.duration()/2);
tl2.pause();

function fireRadio2(){$('.pillar-box[data-num="4"]').find('.radio-group[data-num="2"]').find('.radio-circle').addClass('radio-ping');}
function resetRadio2(){$('.pillar-box[data-num="4"]').find('.radio-group[data-num="2"]').find('.radio-circle').removeClass('radio-ping');}

function turnOnPillars(){
	tl1.seek(0).play();
	tl2.seek(50).play();
}
function turnOffPillars(){
	tl1.pause();
	tl2.pause();
}




//! - SECTION: 4a MODEL

var modelW = 320;
var modelMax = 320;
var modelGap = 120;
var modelCur = 0;
var nextCur = 0;
var totModelSlides = $('.model-slide').length;
var modelRow = 3;
var totModelGroups = Math.ceil(totModelSlides/modelRow);
var slideWinW = 1200;
var modelPer = .3;

$('.model-slide').each(function(i){
	$(this).attr('data-num',(i+1));
})

function initModelSS(){
	modelCur = 0;
	nextCur = 0;
	updateModelW();

	// reset counter/arrows
	$('#model').find('.arrow-btn.left').addClass('off');
	$('#model').find('.count-current').text('01');
}

function updateModelW(){
	// update variables based on screen size

	// desktop
	if(winW>980){
		modelRow = 3;
		totModelGroups = Math.ceil(totModelSlides/modelRow);
		modelPer = .3;

	// tablet
	} else if(winW<=980 && winW>650){
		modelRow = 2;
		totModelGroups = Math.ceil(totModelSlides/modelRow);
		modelPer = .44;

	// mobile
	} else if(winW<=650){
		modelRow = 1;
		totModelGroups = totModelSlides;
		modelPer = .8;
	}

	// get slide container width
	slideWinW = $('.model-slides').find('.contentContainer').width();
	//console.log('--- '+slideWinW)

	// figure out slide width
	modelW = slideWinW*modelPer;
	if(winW>980){
		if(modelW>modelMax){modelW = modelMax;}
	}
	$('.model-slide').width(Math.round(modelW));

	// figure out gap width
	if(winW>980){modelGap = (slideWinW-(modelRow*modelW))/2;}
	if(winW<=980 && winW>650){modelGap = slideWinW-(modelRow*modelW);}
	if(winW<=650){modelGap = 40;}
	vdivX = -(Math.round(modelGap/2));
	$('.model-slide').find('.vdiv').css({'left':vdivX+'px'})

	// add together for total
	totalModelW = Math.ceil((totModelSlides*modelW)+((totModelSlides-1)*modelGap));
	$('.model-slides-wrap').width(totalModelW);

	// change counter
	if(totModelGroups<10){totCt = '0'+(totModelGroups)} else {totCt = totModelGroups;}
	$('#model').find('.count-total').text(totCt);

	// adjust model position
	if(modelCur > 0){
		updateModelP();
	}
}

function updateModelP(){
	newPos = (slideWinW*modelCur)+(modelGap*modelCur);
	if(winW<=650){newPos = (modelW*modelCur)+(modelGap*modelCur);}
	TweenMax.set($('.model-slides-wrap'), {x:-newPos});
}

$('#model').find('.arrow-btn').click(function(){
	if($(this).hasClass('left')){
		dir = -1;
	} else {
		dir = 1;
	}
	changeModelSlider(dir);
})

function changeModelSlider(dir){

	// change active num, get new offset position
	modelCur += dir;
	nextCur = modelCur;
	newPos = (slideWinW*modelCur)+(modelGap*modelCur);
	if(winW<=650){newPos = (modelW*modelCur)+(modelGap*modelCur);}

	// slide over
	TweenMax.to($('.model-slides-wrap'), 1, {x:-newPos, ease:Power3.easeInOut});

	// turn on active sides
	modelSet = modelRow*modelCur;
	$('.model-slide').each(function(i){
		if(i>=modelSet && i<(modelSet+modelRow)){
			$(this).removeClass('off');
		} else {
			$(this).addClass('off');
		}
	})


	// special adjustments
	if(modelCur == 0){
		$('#model').find('.arrow-btn.left').addClass('off');
	} else if($('#model').find('.arrow-btn.left').hasClass('off')) {
		$('#model').find('.arrow-btn.left').removeClass('off');
	}

	if(modelCur == (totModelGroups-1)){
		$('#model').find('.arrow-btn.right').addClass('off');
	} else if($('#model').find('.arrow-btn.right').hasClass('off')) {
		$('#model').find('.arrow-btn.right').removeClass('off');
	}

	// change counter
	if((nextCur+1)<10){showCur = '0'+((nextCur+1))} else {showCur = (nextCur+1);}
	$('#model').find('.count-current').text(showCur);

}





//! - SECTION: 4b MODEL SWIPE (MOBILE)

var msActive = 0;

function updateModelSwipe(){
	totalModelW = Math.ceil((totModelSlides*modelW)+((totModelSlides-1)*modelGap));
	$('.model-slides-wrap').width(totalModelW);

	// position timeline if moved
	msX = -(msActive*modelW);
	TweenMax.set('#model-slider', {'transform':'translate3d('+msX+'px,0,0)'})

	msGridW = (modelW+modelGap);
	msBoundX = totalModelW-modelW;

	// update draggable instance for mobile
	if(draggable){
		updateModelSwipeBounds();
	}
}

var draggableOn = false;
var draggable;

function buildModelSwiper(){

	Draggable.create("#model-slider", {
		type:"x",
		cursor:"move",
		throwProps:true,
		zIndexBoost:false,
		edgeResistance:0.65,
		allowNativeTouchScrolling: true,
		bounds: {minX:-msBoundX, maxX:0, minY:0, maxY:0},

		snap: {
	        x: function(endValue) {
	            return Math.round(endValue / msGridW) * msGridW;

	        },
	    },
	    onThrowComplete: function(){
		    updateMSActive(draggable.x);
	    }
	});
	draggable = Draggable.get("#model-slider");
	draggableOn = true;
}

function updateModelSwipeBounds(){
	draggable.applyBounds({minX:-msBoundX, maxX:0, minY:0, maxY:0});
}

function removeModelSwiper(){
	TweenMax.set($('#model-slider'), {x:0});
	draggable.kill();
	draggableOn = false;
	initModelSS();
}

function updateMSActive(endX){
	msActive = Math.round(-endX/(modelW+modelGap));
	modelCur = msActive;
	nextCur = msActive;
	updateModelArrows();
}

function createModelSwiper(){
	msActive = 0;
	modelCur = 0;
	updateModelSwipe();
	buildModelSwiper();
}

function updateModelArrows(){
	if(modelCur == 0){
		$('#model').find('.arrow-btn.left').addClass('off');
	} else if($('#model').find('.arrow-btn.left').hasClass('off')) {
		$('#model').find('.arrow-btn.left').removeClass('off');
	}

	if(modelCur == (totModelGroups-1)){
		$('#model').find('.arrow-btn.right').addClass('off');
	} else if($('#model').find('.arrow-btn.right').hasClass('off')) {
		$('#model').find('.arrow-btn.right').removeClass('off');
	}

	// change counter
	if((nextCur+1)<10){showCur = '0'+((nextCur+1))} else {showCur = (nextCur+1);}
	$('#model').find('.count-current').text(showCur);
}






//! - SECTION: 5 APPLICATIONS

var app_paths = [];
if ($('#app-path1').length) {
	app_paths[0] = MorphSVGPlugin.pathDataToBezier("#app-path1");
}

if ($('#app-path2').length) {
	app_paths[1] = MorphSVGPlugin.pathDataToBezier("#app-path2");
}

if ($('#app-path3').length) {
	app_paths[2] = MorphSVGPlugin.pathDataToBezier("#app-path3");
}

if ($('#app-path4').length) {
	app_paths[3] = MorphSVGPlugin.pathDataToBezier("#app-path4");
}

if ($('#app-path5').length) {
	app_paths[4] = MorphSVGPlugin.pathDataToBezier("#app-path5");
}

if ($('#app-path6').length) {
	app_paths[5] = MorphSVGPlugin.pathDataToBezier("#app-path6");
}

var app_offsets = [0,1,1.6,.3,.8,1.2];

var app_tls = [];

$.each(app_paths, function(i){
	app_tls[i] = new TimelineMax({repeat:-1, yoyo:true});
	app_tls[i].set('.print-line[data-num="'+(i+1)+'"] .dot', {xPercent:-50, yPercent:-50});
	app_tls[i].to('.print-line[data-num="'+(i+1)+'"] .dot', 2, {bezier: {values:app_paths[i], type:"cubic"}, ease:Quad.easeInOut});
	app_tls[i].pause();
})

function turnOnApp(){
	$.each(app_paths, function(i){
		app_tls[i].seek(app_offsets[i]).play();
	})
}
function turnOffApp(){
	$.each(app_paths, function(i){
		app_tls[i].seek(app_offsets[i]).pause(0, true);;
	})
}





//! - SECTION: 9 TEAM

var teamRow = 3;
var teamSt = 4;
var teamM;

function updateTeam(){
	if(winW>550){

		if(winW<=800){
			teamRow = 2;
			teamSt = 3;
		} else {
			teamRow = 3;
			teamSt = 4;
		}

		teamM = ($('.team-grid').width()-($('.team-box').width()*teamRow))/(teamRow-1)
		$('.team-box:nth-child(n+'+teamSt+')').css({'margin-top':teamM+'px'})

	} else {
		$('.team-box').css({'margin-top':''})
	}
}





//! - ANIMATION: HELPERS

$('.radio-group').append('<div class="stand"></div><div class="radio-circle" data-num="1"></div><div class="radio-circle" data-num="2"></div><div class="radio-circle" data-num="3"></div><div class="ball"></div>');

function addStatic(elem, amt){
	tmpS = '';
	for(i=0;i<amt;i++){
		tmpS += '<div class="static-line" data-num="'+(i+1)+'"><div class="line"></div></div>';
	}
	elem.append(tmpS);
}

// add static

addStatic($('#hero').find('.static-lines'), 20);
$('h2.hasStatic').prepend('<span class="static-lines"></span>');
addStatic($('h2.hasStatic').find('.static-lines'), 10);

// offset radio pings

radioGap = .6;
radioOffset = 2;

function offsetPings(elem, offset){
	if(offset == 0){os = radioOffset;} else {os = offset;}

	elem.find('.radio-group').each(function(i){
		rgap = radioGap;
		if($(this).hasClass('light')){rgap = .75;}

		$(this).find('.radio-circle').each(function(n){
			tmpDel = (os*i)+(rgap*n);
			$(this).css({'animation-delay':-tmpDel+'s'})
		})
	})
}

// offset radio pings
offsetPings($('#what'), .75);
offsetPings($('#model'), 0);
offsetPings($('#token'), 2);
offsetPings($('#globalFooter'), 0);

// offset static lines
$('#hero').find('.static-line').each(function(){
	randomOffsets($(this).find('.line'), 4);
})
$('h2.hasStatic').find('.static-line').each(function(){
	randomOffsets($(this).find('.line'), 4);
})





//! - ANIMATION: SCROLLMAGIC

var hero_scene1;

function initScrollMagic(){

var controller = new ScrollMagic.Controller();

$('.hasAnim').each(function(){
    var currentElem = '#'+$(this).attr('id');
    //console.log(currentElem)
    var scene = new ScrollMagic.Scene({triggerElement: currentElem, triggerHook: 2, duration: winH+$(currentElem).outerHeight()})
        .addTo(controller);
        scene.setClassToggle(currentElem, "on");
});

	// pillars section call function

	var pillarsScene = new ScrollMagic.Scene({
			triggerElement: "#pillars",
			triggerHook: 2,
			duration: winH+$('#pillars').outerHeight()
		})
		.on('enter',function(){
			turnOnPillars();
		})
		.on('leave',function(){
			turnOffPillars();
		})
		.addTo(controller);


	// applications section call function

	var appScene = new ScrollMagic.Scene({
			triggerElement: "#applications",
			triggerHook: 2,
			duration: winH+$('#applications').outerHeight()
		})
		.on('enter',function(){
			turnOnApp();
		})
		.on('leave',function(){
			turnOffApp();
		})
		.addTo(controller);

}

var scrollMagicOn = true;
initScrollMagic();







