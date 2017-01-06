#include "InputStructs.hlsli"

[maxvertexcount(3)]
void main(
	triangle PInput input[3], 
	inout TriangleStream< PInput > output
)
{
	float3 U = input[1].WorldPosition.xyz - input[0].WorldPosition.xyz;
	float3 V = input[2].WorldPosition.xyz - input[0].WorldPosition.xyz;

	float3 vNormal = normalize( cross( U, V ) );

	for (uint i = 0; i < 3; i++)
	{
		PInput element = input[i];
		element.Normal = vNormal;
		output.Append(element);
	}
}