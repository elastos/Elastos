var mobile = false;
var mobileBreak = 750;
if($(window).width() <= mobileBreak){mobile = true;}
var is_firefox = navigator.userAgent.toLowerCase().indexOf('firefox') > -1;
var path;
var isHome = false;
if($('body').attr('id') == 'page-home'){isHome = true;}
function freezePage(){$('body').css({'width':'100%','height':'100%','overflow':'hidden'});}
function unfreezePage(){$('body').css({'width':'','height':'','overflow':''});}
function animScroll(sec, speed, offset){
	activeOffset = $(sec).offset().top+offset;	
	TweenMax.to('html,body', speed, {scrollTop:activeOffset, ease:Expo.easeInOut});
}





/*! - GLOBAL ***************************** */





//! - GLOBAL: 0 RESIZE

var winW;
var winH;
$(window).resize(function(){
	winW = $(window).width();
	winH = $(window).height();
	//console.log(winW+' / '+winH);
	
	// go to mobile
	if(winW<=mobileBreak && !mobile){
		mobile = true;
		
		if(stickyOpen){
			$('#stickyBar').removeClass('open');
			stickyOpen = false;
		}
		
		if($('.menu-btn').hasClass('on')){
			$('.menu-btn').removeClass('on');
		}
		
	}
	
	// go to desktop
	if(winW>mobileBreak && mobile){
		mobile = false;
		if(sT>=menuBtnT && !$('.menu-btn').hasClass('on')){
			$('.menu-btn').addClass('on');
		}
	}
	
	// update scroll top positions
	if(isHome){
		menuBtnT = $('#welcome').offset().top - $('#hero').outerHeight()/2;
		footerT = $('#globalFooter').offset().top-500;
	}
})
$(window).resize();





/*! - SCROLLING ***************************** */





//! - SCROLLING: 0 STICKY ELEMENTS

var sT;
var stickyOpen = false;
var stickyBreak = 750;
var stickyH = 54;
var lastSt = 0;
var autoOff = false;

$(window).on("scrollstart",function(){
	scroll_interval = setInterval(function(){			
	
		sT = $(this).scrollTop();
		
		// set sticky bar
		setSticky();
		
		// turn on menu
		if(winW>stickyBreak){
			setMenuBtn();	
		}
		
	}, 10);
})
$(window).on("scrollstop",function(){
	if(scroll_interval){
		clearInterval(scroll_interval);
	}
})

function setSticky(){
	
	// drop sticky bar on scroll up
	if(sT<lastSt && winW>=stickyBreak){	
		if(!stickyOpen && sT > 110){
			$('#stickyBar').addClass('open');
			stickyOpen = true;
		}	
		if(stickyOpen && sT <= 0){
			$('#stickyBar').removeClass('open');					
			stickyOpen = false;
		}
	}
	
	// close sticky bar on scroll down
	if(sT>lastSt){
		if(stickyOpen){
			$('#stickyBar').removeClass('open');					
			stickyOpen = false;
			
			// if filter open, close it
			$('.hasFilter').removeClass('open');
		}
	}
	
	lastSt = sT;
}

if(isHome){
	var menuBtnT = $('#welcome').offset().top - $('#hero').outerHeight()/2;
	var footerT = $('#globalFooter').offset().top-500;
}

function setMenuBtn(){
	// bring on at top
	if(sT>=menuBtnT && !$('.menu-btn').hasClass('on')){
		$('.menu-btn').addClass('on');
	}
	if(sT<menuBtnT && $('.menu-btn').hasClass('on')){
		$('.menu-btn').removeClass('on');
	}
	
	// remove at footer
	if(sT>=footerT && $('.menu-btn').hasClass('on')){
		$('.menu-btn').removeClass('on');
	}
	if(sT<footerT && sT>menuBtnT && !$('.menu-btn').hasClass('on')){
		$('.menu-btn').addClass('on');
	}
}





//! - SCROLLING: 1 SCROLLMAGIC

function initScrollMagic(){
	
var controller = new ScrollMagic.Controller();	
	
$('.hasAnim').each(function(){
    var currentElem = '#'+$(this).attr('id');
    //console.log(currentElem)
    var scene = new ScrollMagic.Scene({triggerElement: currentElem, triggerHook: 2, duration: winH+$(currentElem).outerHeight()})
        .addTo(controller);
        scene.setClassToggle(currentElem, "on");
});

}

if(isHome){
	initScrollMagic();
}




//! - GLOBAL: 3 FORMS

var formSent = false;

$('.global-form').submit(function(){
	if(validateForm($(this))){
		sendForm($(this));
	}
	return false;
});

function sendForm(formObj){

// animation actions

var formURL = $(formObj).attr('action');
var formData = $(formObj).serialize();

$(formObj).find('button').attr('disabled','disabled');

$.ajax({
    url: formURL,
    type: 'GET',
    data: formData,
	dataType: "jsonp",
    jsonp: "c",
    contentType: "application/json; charset=utf-8",
        
    success: function(data){	
	    // testing
	    /*console.log(data.result);
	    if(data.result == 'error'){
		    console.log(data.msg);
	    }*/
	    				
		formSent = true;
		TweenMax.to($(formObj).find('.form-msg'), .5, {delay:1.5, opacity:1, 'visibility':'visible', onComplete:function(){			
			tmpid = $(formObj).attr('id');
			document.getElementById(tmpid).reset();
			formSent = true;
			formSending = false;
			$(formObj).find('button').removeAttr('disabled');
		}})	
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
		$(formObj).addClass('error');
		return false;
	}
}

$('[data-type="req"]').focus(function(){
	if($(this).parents('form').hasClass('error')){
		$(this).parents('form').removeClass('error')
	}
})
$('.global-form').find('input').focus(function(){
	if(formSent){
		TweenMax.to($(this).parents('.global-form').find('.form-msg'), .5, {opacity:0})	
		formSent = false;
	}
})





//! - GLOBAL: 4 FILTER

$('.hasFilter>a').click(function(){
	if($(this).parents('.hasFilter').hasClass('open')){
		$(this).parents('.hasFilter').removeClass('open');
		$(this).parents('.menu-contents.mob').children('.langFilter').removeClass('open');
	} else {
		$(this).parents('.hasFilter').addClass('open');
		$(this).parents('.menu-contents.mob').children('.langFilter').addClass('open');
		
		// click anywhere to close filter
		if(winW>mobileBreak){
			setTimeout(function(){			
				$('body').bind('click', function(){
					$('.hasFilter').removeClass('open');
					$('body').unbind('click');
				})		
			}, 100)
		}
	}
	return false;
})





//! - GLOBAL: 2 LOADER

var loadReady = false;
var loadSp = .58;
var loadDel = .18;
var topDel = loadDel*6;
var endDel = ((topDel*2)*1000) - 200;
var loadEase = 'Power3.easeInOut';
	
freezePage();

function animateLogo(){	
	
	// top
	for(i=0;i<6;i++){	
		trg = $('.tri-group.top').find('.tri[data-num="'+(i+1)+'"]')	
		if(i == 2 || i == 4){trg = $('.tri-group.top').find('.tri[data-num="'+(i+1)+'"]>div');}
		
		TweenMax.to(trg, loadSp, {delay:(i*loadDel), startAt:{'display':'block'}, opacity:1, rotationY:0, ease:loadEase});			
	}
	
	// bottom
	for(i=0;i<6;i++){	
		trg = $('.tri-group.bot').find('.tri[data-num="'+(i+1)+'"]')	
		if(i == 2 || i == 4){trg = $('.tri-group.bot').find('.tri[data-num="'+(i+1)+'"]>div');}
		
		TweenMax.to(trg, loadSp, {delay:(i*loadDel)+topDel, startAt:{'display':'block'}, opacity:1, rotationY:0, ease:loadEase});			
	}
	
	// set for listener to open page
	setTimeout(function(){
		loadReady = true;
	}, endDel)
	
}

// set cookie for loader viewing

var visited = false;

if(isHome){
	if(document.cookie.indexOf('visited=') == -1){
		
		// first time on page, show loader
		setTimeout(function(){animateLogo()}, 500);
		
		// set cookie
		document.cookie = "visited=yes";
		
	} else {
		visited = true;
		loadReady = true;
	}
} else {
	loadReady = true;
}

$(window).on('load', function(){
	
	loadChecker = setInterval(function(){
		if(loadReady){
			// slide over
 			TweenMax.to('.load-clip', .6, {delay:.2, x:0, ease:Power3.easeInOut})
 			TweenMax.to('.logo-text', .4, {delay:.4, 'width':'259px', opacity:1, ease:Power3.easeInOut})
 			
 			loadSC = .8; if(winW<=650){loadSC = .4;}

 			TweenMax.to('.load-clip', .5, {delay:.8, opacity:0, scaleX:loadSC, scaleY:loadSC, ease:Power3.easeInOut})
 			
 			loadDel = 0;
 			if(isHome && !visited){loadDel = 1;}
 			
 			TweenMax.to('#loader', .5, {delay:loadDel, opacity:0, 'display':'none', onComplete:function(){
 				unfreezePage();
 			}});
			$(window).resize();
			clearInterval(loadChecker);
		}
	}, 30)
	
})





//! - GLOBAL: 1 MENU

var menuOpen = false;
var closingMenu = false;

$('.menu-btn').click(function(){		
		
	if(!menuOpen){
		
		// set for menu view
		$('#globalMenu').addClass('open expanded');

		// animate open
		TweenMax.set('#menu-scroll', {scrollTop:0})
		TweenMax.to('.menu-wrap', .75, {opacity:1, 'display':'block', onComplete:function(){freezePage();}})
		
		// animate on items
		$('#globalNav').find('li').each(function(i){
			TweenMax.set($(this), {opacity:0, y:10})
			TweenMax.to($(this), .5, {delay:(i*.15), y:0, opacity:1, ease:Power3.easeOut})
		})
							
		menuOpen = true;
	} else if(!closingMenu) {			
		closeMenu();	
	}
})

function closeMenu(){
	closingMenu = true;
	
	$('#globalMenu').removeClass('open');
	
	TweenMax.to('.menu-wrap', .5, {opacity:0, 'display':'none', onComplete:function(){
		unfreezePage();
		$('.menu-wrap').hide();
		$('#globalMenu').removeClass('expanded');
		closingMenu = false;
	}})			

	menuOpen = false;
}

// nav items go to section; close menu
$('#globalNav').find('a').click(function(){
	tmpID = $(this).attr('href');
	tmpOff = 0;
	if($(tmpID).attr('data-offset')){
		tmpOff = $(tmpID).attr('data-offset');
	}
	closeMenu();
	
	setTimeout(function(){
		animScroll($(tmpID), .75, tmpOff);
	}, 550)
	
	return false;
})

if(isHome){
	$('#stickyBar').find('.logo').click(function(){
		animScroll($('body'), .75, 0);
	})
}








//! - VIDEO OVERLAY

lb_video = document.getElementById("lbvid");

function openVideoOverlay(){
	vid = 'Elastos';
	freezePage();

	$('#video-overlay').css({'display':'block'});
	TweenMax.to($('#video-overlay'), .5, {opacity:1});
	
	lb_video.pause();
	
	vidHTML = '<source src="video/' + vid + '.webm" type="video/webm">';
	vidHTML += '<source src="video/' + vid + '.ogv" type="video/ogg">';
	vidHTML += '<source src="video/' + vid + '.mp4" type="video/mp4">';
	
	$('#lbvid').html(vidHTML);
	$('#lbvid').attr('poster',vid+'.jpg');
	lb_video.load();
	
	lb_video.addEventListener('loadedmetadata', function() {
		lb_video.play();
	}, false);	
}

function closeVideoOverlay(){
	
	TweenMax.to($('#video-overlay'), .5, {opacity:0, 'display':'none', onComplete:function(){
		lb_video.pause();
		unfreezePage();
	}});
	
}

$('#video-overlay').click(function(){
	closeVideoOverlay();
})

$('#lbvid').click(function(e){
	e.stopPropagation();
})

$('.play-btn').click(function(){
	openVideoOverlay();	
	return false;
})





/*! - SECTION ***************************** */





//! - SECTION: 4 SEGMENTS

var curSeg = 0;
var barW = 150;
var totalSegs = 4;
var changeInt = 750;

function changeSegment(){
	curTrg = $('.seg-next-bar[data-num="'+curSeg+'"]');
	nxtTrg = $('.seg-next-bar[data-num="'+nextSeg+'"]');
	
	// adjust for responsive
	if(winW<=1200){barW = 100;}
	if(winW<=1024){barW = 60;}
	
	// cover contents with current segment bar
	curTrg.addClass('changing');
	curTrg.find('.bar-cover').removeClass('off');
	startSc = barW/winW;
	
	TweenMax.to(curTrg.find('.bar-cover'), .75, {startAt:{scaleX:startSc}, scaleX:1, ease:Power3.easeInOut});
	TweenMax.to(curTrg.find('.title-wrap'), .5, {delay:.25, opacity: 0});
	
	// change out contents
	setTimeout(function(){
		$('.change-wrap[data-num="'+curSeg+'"]').hide();
		$('.change-wrap[data-num="'+nextSeg+'"]').show();
		$('.seg-benefits-bg').attr('data-num',nextSeg);
	}, changeInt)
	
	// remove last segment cover
	setTimeout(function(){
		curTrg.find('.bar-cover').addClass('off');
		TweenMax.to(curTrg.find('.bar-cover'), .75, {scaleX:0, ease:Power3.easeInOut});
		
		// bring on new sidebar
		TweenMax.set(nxtTrg.find('.title-wrap'), {opacity: 1});
		TweenMax.set(nxtTrg, {'display':'none'});
		TweenMax.to(nxtTrg, .75, {delay:.5, startAt:{'display':'block', x:barW}, x:0, ease:Power3.easeInOut, onComplete:function(){
			curTrg.removeClass('changing');
			TweenMax.set(curTrg, {'display':'none'});
		}});
		
		// update active number
		curSeg = nextSeg;
	}, changeInt+100)
	
	// update slide controls
	setTimeout(function(){
		
		// update slide counter
		$('.count-current').text('0'+(nextSeg+1));
		
		// update arrows
		if(nextSeg == 0){$('#segments').find('.arrow-btn.left').addClass('off');}
		if(nextSeg > 0){$('#segments').find('.arrow-btn.left').removeClass('off');}
		if(nextSeg == (totalSegs-1)){$('#segments').find('.arrow-btn.right').addClass('off');}
		if(nextSeg < (totalSegs-1)){$('#segments').find('.arrow-btn.right').removeClass('off');}
	
	}, changeInt)
	
}

$('.seg-next-bar').click(function(){
	nextSeg = curSeg+1;
	if(nextSeg == totalSegs){nextSeg = 0}
	changeSegment();
})

$('#segments').find('.arrow-btn').click(function(){
	if($(this).hasClass('right')){
		nextSeg = curSeg+1;
	} else {
		nextSeg = curSeg-1;
	}	
	
	changeSegment();
})





//! - SECTION: 5 USE CASES

var scrTxt = '<span>Data Ownership, Privacy & Monetization</span><span>Decentralization</span><span>Digital Assets, Content & Intellectual Property</span><span>Smart Contracts, IoT, Automation, & Provenance</span>';

// init scrolling text

$('.scroll-txt').each(function(){
	$(this).html(scrTxt+scrTxt);
})









