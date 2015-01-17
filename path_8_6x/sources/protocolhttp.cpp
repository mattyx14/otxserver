////////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
////////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////
#include "otpch.h"
#include "protocolhttp.h"

#include "outputmessage.h"
#include "connection.h"

#ifdef __ENABLE_SERVER_DIAGNOSTIC__
uint32_t ProtocolHTTP::protocolHTTPCount = 0;

#endif
#ifdef __DEBUG_NET_DETAIL__
void ProtocolHTTP::deleteProtocolTask()
{
	std::clog << "Deleting ProtocolHTTP" << std::endl;
	Protocol::deleteProtocolTask();
}

#endif
void ProtocolHTTP::onRecvFirstMessage(NetworkMessage&)
{
	if(OutputMessage_ptr output = OutputMessagePool::getInstance()->getOutputMessage(this, false))
	{
		TRACK_MESSAGE(output);

		output->putString("HTTP/1.1 200 OK");
		output->putString("Date: Fri, 27 Mar 2009 17:28.13 GMT\r\n");
		output->putString("Server: The OTX Server httpd/2.X.Series\r\n");
		output->putString("Content-Location: index.html\r\n");
		//Vary: negotiate\r\n
		//TCN: choice\r\n
		output->putString("Last-Modified: Fri, 27 Mar 2009 17:28.13 GMT\r\n");
		output->putString("Accept-Ranges: bytes\r\n");
		output->putString("Content-Length: 1234\r\n");
		output->putString("Expires: Fri, 27 Mar 2009 17:28.13 GMT\r\n");
		output->putString("Connection: close\r\n");
		output->putString("Content-Type: text/html qs=0.7\r\n");
		output->putString("\r\n");
		output->putString("<html><head><title>The OTX Server httpd</title></head><body>It works (apache ripoff ;D)!</body></html>");

		OutputMessagePool::getInstance()->send(output);
	}

	getConnection()->close();
}
