# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

class ConfigurationTestCase(ClientServerTestCase):

    def setupServerSide(self, current):
        # Nothing to do if we're not running this test with the C++ mapping
        if not isinstance(self.getMapping(), CppMapping):
            return

        certsPath = os.path.abspath(os.path.join(self.getPath(), "..", "certs"))
        if isinstance(platform, Darwin) and current.config.buildPlatform == "macosx":
            keychainPath = os.path.join(certsPath, "Find.keychain")
            os.system("mkdir -p {0}".format(os.path.join(certsPath, "keychain")))
            os.system("security create-keychain -p password %s" % keychainPath)
            for cert in ["s_rsa_ca1.p12", "c_rsa_ca1.p12"]:
                os.system("security import %s -f pkcs12 -A -P password -k %s" % (os.path.join(certsPath, cert), keychainPath))
        elif current.config.openssl or platform.hasOpenSSL():
            #
            # Create copies of the CA certificates named after the subject
            # hash. This is used by the tests to find the CA certificates in
            # the IceSSL.DefaultDir
            #
            for c in ["cacert1.pem", "cacert2.pem"]:
                pem = os.path.join(certsPath, c)
                out =  run("{openssl} x509 -subject_hash -noout -in {pem}".format(pem=pem, openssl=self.getOpenSSLCommand()))
                shutil.copyfile(pem, "{dir}/{out}.0".format(dir=certsPath, out=out.splitlines()[0]))

    def teardownServerSide(self, current, success):
        # Nothing to do if we're not running this test with the C++ mapping
        if not isinstance(self.getMapping(), CppMapping):
            return

        certsPath = os.path.abspath(os.path.join(self.getPath(), "..", "certs"))
        if isinstance(platform, Darwin) and current.config.buildPlatform == "macosx":
            os.system("rm -rf {0} {1}".format(os.path.join(certsPath, "keychain"), os.path.join(certsPath, "Find.keychain")))
        elif current.config.openssl or platform.hasOpenSSL():
            for c in ["cacert1.pem", "cacert2.pem"]:
                pem = os.path.join(certsPath, c)
                out =  run("{openssl} x509 -subject_hash -noout -in {pem}".format(pem=pem, openssl=self.getOpenSSLCommand()))
                os.remove("{dir}/{out}.0".format(out=out.splitlines()[0], dir=certsPath))

    def getOpenSSLCommand(self):
        if isinstance(platform, Windows):
            return os.path.join(self.getPath(), "..", "..", "..", "msbuild", "packages", "zeroc.openssl.v140.1.0.2.4",
                                "build", "native", "bin", "Win32", "Release", "openssl.exe")
        else:
            return "openssl"

class IceSSLConfigurationClient(Client):

    def getExe(self, current):
        if isinstance(platform, Windows) and current.config.openssl:
            return "clientopenssl"
        return Client.getExe(self, current)

class IceSSLConfigurationServer(Server):

    def getExe(self, current):
        if isinstance(platform, Windows) and current.config.openssl:
            return "serveropenssl"
        return Server.getExe(self, current)

# Filter-out the deprecated property warnings
outfilters = [ lambda x: re.sub("-! .* warning: deprecated property: IceSSL.KeyFile\n", "", x) ]

#
# With UWP, we can't run this test with the UWP C++ server (used with tcp/ws)
#
options=lambda current: { "protocol": ["ssl", "wss"] } if current.config.uwp else {}

TestSuite(__name__, [
   ConfigurationTestCase(client=IceSSLConfigurationClient(outfilters=outfilters, args=['"{testdir}"']),
                         server=IceSSLConfigurationServer(outfilters=outfilters, args=['"{testdir}"']))
], multihost=False, options=options)
