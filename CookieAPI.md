# Introduction #

Paste these functions into your global function %module, or put them in a separate %included file for that module.   They are helpful.

# Details #

```

%define(cur-cookie
        ,%gset(cur-cookie,%header(Cookie))%cur-cookie%
)

%define(get-cookie,
         %enumtoken(%cur-cookie%,;
                ,%if(%equal(%tolower(%trim(%gettoken(%token%,=,0))),%name%)
                        ,%url-decode(%gettoken(%token%,=,1))%break%
                 )
         )
,name)

%define(set-cookie,'%trim(
        %if(%not(%equal(%invoke(setcook-%name%),%value%))
        ,
        %set(zone,%gettoken(%header(host),:,0))
        %if(%wcmatch(%zone%,*.*.*)
                ,%let(zone,%mid(%zone%,%iadd(1,%pos(.,%zone%))))
        )
        %set(expstr,%if(%expsec%,Expires=%fmtgtime(%iadd(%expsec%,%time%),"www, dd-mmm-yyyy hh:mm:ss") GMT;))
        %gset(cur-cookie,%name%=%url-encode(%value%);%cur-cookie%)
        %gset(setcook-%name%,%value%)
        %append-header(Set-Cookie,"%name%=%url-encode(%value%); %expstr% Path=/; Domain=.%zone%")
        )
),name,value,expsec)

%define(del-cookie,'%trim(
        %set(zone,%gettoken(%header(host),:,0))
        %if(%wcmatch(%zone%,*.*.*)
                ,%let(zone,%mid(%zone%,%iadd(1,%pos(.,%zone%))))
        )
        %rxmatch(;%cur-cookie%,(.*); *%name%=[^;]+(.*),%let(cur-cookie,%subx(1)%subx(2)))
        %append-header(Set-Cookie,"%name%=; Expires=Thu 01-Jan-1971 12:01:00 GMT; Path=/; Domain=%zone%")
),name)

```