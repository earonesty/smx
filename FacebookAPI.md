# Introduction #

2 functions I use so far, and it's all I've needed to use facebook's connect api for single-signon and to get some user details, friend lists, etc.

You'll want to set %fb-appkey% and %fb-secret% as globals to use these, or you can wrap them in a context (IE: %facebook(%connect%))

%fb-connect% returns "true" if the current "facebook connect" session is logged in.   You'll need to implement the xd\_receiver thingy, and after that facebook will hand you a buch of third party cookies.  This function validates them.   If %fb-user% is set... you're logged in.

%fb-api(method, params) issues a REST call to facebooks server and computes the signature.  Works great.

# Details #

```
%define(fb-connect
        ,
        %nil(
         %if(%equal(%md5(
            %enumsort(%cur-cookie%,;
                ,%set(name
                        ,%tolower(%trim(%gettoken(%token%,=,0)))
                 )%if(%wcmatch(%name%,%fb-appkey%_*)
                        ,%mid(%name%,%iadd(%len(%fb-appkey%),1))=%url-decode(%gettoken(%token%,=,1))
                 )
            )%fb-appsecret%),%get-cookie(%fb-appkey%))
                ,
                 %gset(fb-expires,%get-cookie(%fb-appkey%_expires))
                 %gset(fb-session_key,%get-cookie(%fb-appkey%_session_key))
                 %gset(fb-ss,%get-cookie(%fb-appkey%_ss))
                 %gset(fb-user,%get-cookie(%fb-appkey%_user))
                 %out(1)
                ,%gset(fb-user,)
         )
        )
)

%define(fb-api,%trim(
        %set(params,%params%&method=%method%&api_key=%fb-appkey%&call_id=%time%&v=1.0)
        %set(sigp,%enumsort(%params%,&,
                %token%
        ))
        %set(sig,%md5(%sigp%%fb-appsecret%))
        %set(url,http://apps.facebook.com/restserver.php?%params%&sig=%sig%)
        %include(%url%)
),method,params)
```