
var TargetPage=null;

function OnInit()
{
	TranslatePage();

	TargetPage=GetQueryString("target");
	
	// Orca: fallback timeout in case the C++ -> JS signal fails (e.g., WebKit issues).
	// Bounded fallback for WebView implementations that lose the native callback.
	setTimeout("JumpToTarget()",30*1000);

	var tSend={};
	tSend['sequence_id']=Math.round(new Date() / 1000);
	tSend['command']="userguide_loading_page_ready";
	SendWXMessage(JSON.stringify(tSend));
}

function HandleStudio( pVal )
{
	let strCmd=pVal['command'];
	
	if(strCmd=='userguide_profile_load_finish')
	{
		JumpToTarget();
	}
}

function JumpToTarget()
{
	window.open('../'+TargetPage+'/index.html','_self');
}
