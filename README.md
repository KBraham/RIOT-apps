Personal RIOT apps


# Native build guidelines

__Create local tap bridge__

```
../../RIOT/dist/tools/tapsetup/tapsetup -c 2
```

__Create local ipv6 network__

RADVD config

```
interface tapbr0
{
	AdvSendAdvert on;
	MinRtrAdvInterval 30;
	MaxRtrAdvInterval 100;
	prefix fd77:d53:6c7c::/64
	{
		AdvOnLink off;
		AdvAutonomous on;
		AdvRouterAddr on;
	};

	abro fe80::e8e2:5dff:fe6e:93fd
	{
        AdvVersionLow 10;
        AdvVersionHigh 2;
        AdvValidLifeTime 2;
	};
};


 sudo radvd -d 5 -m stderr -n
```

__Start native emulator__

```
make all term PORT=tap0
```
