#Requires -Modules Pester

Param(
	[ValidateSet("Lex", "Parse")]
	$Type = "Parse",  # "Lex",
	
	[Switch]
	$Rebuild,
	
	[Switch]
	$Confirm,
	
	[Switch]
	$WhatIf,
	
	[Switch]
	$Debug,
	
	[Switch]
	$NoStub,
	
	[Switch]
	$GetBuildString
)

Import-Module Pester

$ErrorActionPreference = "Stop"

$cmd = "g++"
$out = "demo.exe"
$src = "./src/*.cpp"
$include = "./include"
$expression = "$cmd $src -o $out -O3 -std=c++17 -I$include"

if ($Debug) {
	$expression = "$expression -DDEBUG"
}

if ($GetBuildString) {
	return $expression
}

if ($Rebuild) {
	if ($WhatIf) {
		Write-Output $expression
		Write-Output ""
	} else {
		iex $expression
	}
}

$tests = @(
	, @{"In"= ""; "Out"= $null}
	, @{"In"= "2"; "Out"= 2}
	, @{"In"= "-2"; "Out"= -2}
	, @{"In"= "2 + 3"; "Out"= 5}
	, @{"In"= "2 + 3 * 2"; "Out"= 8}
	, @{"In"= "1 + 2 * 3 + 4 * 5 + 6"; "Out"= 33}
	, @{"In"= "1 + 2 * (3 + 4) * 5 + 6"; "Out"= 77}
	, @{"In"= "1 + 2 * ((3 + 4) * 5 + 6)"; "Out"= 83}
	, @{"In"= "-1 + -2 * -(-(-3 + -4) * -5 + -6)"; "Out"= -83}
	, @{"In"= "-1 + -2 * -(-(-3 + -(-4)) * -5 + -6)"; "Out"= -3}
	, @{"In"= "-1.019 + 2.021 * -(-(-3.17 + -(-4.3)) * -5 + -.131313)"; "Out"= -12.172266427}
	, @{"In"= "-0.5 * ( 2 + sin( 29 ) ) "; "Out"= -0.668183057893516}
	, @{"In"= "-0.5*(2+sin(29)) + 3"; "Out"= 2.33181694210648}
	, @{"In"= "sin(144)^2 + cos(144)^2"; "Out"= 1}
	, @{"In"= "min(20, 2) * exp(max(1.12, -2.13))"; "Out"= 6.129708406586005}
	, @{"In"= "what(1.12, -2.13)"; "Out"= "Error: no definition found with the name 'what'"}
	, @{"In"= "abs(2, 3, 4)"; "Out"= "Error: no overload found for definition 'abs' that takes 3 arguments"}
)

$Type = $Type.ToLower()

$epsilon = 0.00000000000001

$tests | % {
	if (-not $NoStub) {
		echo "Testing [$($_.In)]`r`n"
	}
	
	$expression = ".\$out $Type `"$($_.In)`""
	
	if ($WhatIf) {
		Write-Output $expression
	} else {
		if ($Debug -or $Type -eq "Lex") {
			iex $expression
			
			if (-not $NoStub) {
				Write-Output ""
				Write-Output "    Expected: $($_.Out)"
			}
		}
		else {
			$actual = iex $expression 2>&1
			
			if (-not $NoStub) {
				Write-Output "    Expected: $($_.Out)"
				Write-Output "    Actual:   $actual"
				Write-Output ""
			}
			
			if ($actual -ne $null) {
				Write-Output $actual.GetType().Name
			}
			
			Describe ".\$out $Type `"$($_.In)`"" {
				It "passes" {
					if ($_.Out -eq $null) {
						$actual | Should Be $null
					}
					elseif ($_.Out.GetType() -eq [Double]) {
						([Math]::Abs($actual - $_.Out)) | Should BeLessThan $epsilon
					}
					else {
						$actual | Should Be $_.Out
					}
				}
			}
		}
	}
	
	echo ""
	
	if ($Confirm) {
		[System.Console]::ReadKey() | Out-Null
	}
}
