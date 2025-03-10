// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

struct SDKConfig
{
	/** The product id for the running application, found on the dev portal */
	static constexpr char ProductId[] = "633554ceffec48769c78fe3f7452cb52";

	/** The sandbox id for the running application, found on the dev portal */
	static constexpr char SandboxId[] = "e7cddbce5a2343f99457e3b445d6845c";

	/** The deployment id for the running application, found on the dev portal */
	static constexpr char DeploymentId[] = "191f4a96779f4b97affbf014df6b5b2d";

	/** Client id of the service permissions entry, found on the dev portal */
	static constexpr char ClientCredentialsId[] = "xyza78919bGuLAoFOcvNepOD6y0FaLQs";

	/** Client secret for accessing the set of permissions, found on the dev portal */
	static constexpr char ClientCredentialsSecret[] = "Zh5SLxWWlv3czh6Ft6yvd9u6ZV901u22ysF51u++b/w";

	/** Game name */
	static constexpr char GameName[] = "swamptgame";

	/** Encryption key */
	static constexpr char EncryptionKey[] = "1111111111111111111111111111111111111111111111111111111111111111";
        
	/** Credential name in the DevAuthTool */
	static constexpr char CredentialName[] = "swampcred";

	/** Host name in the DevAuthTool */
	static constexpr char Port[] = "localhost:1024";
};
