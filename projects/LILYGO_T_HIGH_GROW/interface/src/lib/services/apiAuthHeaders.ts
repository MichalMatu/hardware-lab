export function apiAuthHeaders(
	securityEnabled: boolean,
	bearerToken: string
): Record<string, string> {
	if (!securityEnabled) return {};
	return { Authorization: `Bearer ${bearerToken}` };
}
