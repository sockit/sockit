    function scrollDown() {
	    $("#out").scrollTop(10000000);
    }


	function output(s)
	{
		$("#out").html( $("#out").html() + "<p>" + s + "</p>");
		console.log(s);
        scrollDown();
	}

	function mils()
	{
		return (new Date()).getTime();
	}

    function isprime(n)
    {
        for(var d = 2; d < n; d++)
        {
            if(n % d == 0)
            {
                return 0;
            }
        }
        return n;
    }

    function taketime()
    {
        var len = 100000;
        var max = 0;

        for(var i = 0; i < len; i++)
        {
            p = isprime(i);

            if(p > max)
            {
                max = p
            }
        }

		console.log("done");
    }

    function filter(f, arr)
    {
        var len = arr.length;
        var newarr = new Array();
        for(var i = 0; i < len; i++)
        {
            if(f(arr[i]))
            {
                newarr.push(arr[i])
            }
        }

        return newarr;
    }

    function filter_cps(f, arr, k)
    {
        if(arr.length > 0)
        {
            var head = arr.shift();
            return filter_cps(f, arr, (function(a) {
                        if (f(head)) {
                            a.unshift(head)
                            }
                        return k(a);
                        }) );
        }

        if(arr.length == 0)
        {
            return k([]);
        }

        return k;
    }

    function test()
    {
        var arr = [2,3,4,5,6,7,8,9,10];
        var primes = filter(isprime, arr);
        alert(primes);
        return primes;
    }

    function test_cps()
    {
        var arr = [2,3,4,5,6,7,8,9,10];
        var primes = filter_cps(isprime, arr, (function(x) { return x; }));
        console.log(primes);
        return primes;
    }

	function times(n)
	{
		var x = 0;
		return function()
		{
			x += 1

			if(x % n == 0)
			{
				output(x);
			}
		};
	}
