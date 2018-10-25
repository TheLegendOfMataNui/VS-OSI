using System;
using System.Collections.Generic;
using System.Diagnostics;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace OSIProject.Language.Test
{
    [TestClass]
    public class UnitTest1
    {
        private const string testFragment1 = @"; Generator: @sage-js/cli: 0.9.1
; SHA256: 40B7915E02CF2E5B51CE1ED64B829B83C39635E5724EBCC8276DE9306AC02E90

begin metadata
    version 4, 1
end ; metadata

begin strings
    string "" ""                                                                                                                                                     ; 0

    string ""\n""                                                                                                                                                    ; 1
    string ""Root/Data/Strings/""                                                                                                                                    ; 2
    string ""English.tbl""                                                                                                                                           ; 3
    string ""German.tbl""                                                                                                                                            ; 4
    string ""French.tbl""                                                                                                                                            ; 5
    string ""Danish.tbl""                                                                                                                                            ; 6
    string ""Swedish.tbl""                                                                                                                                           ; 7
    string ""Norwegian.tbl""              ";

        [TestMethod]
        public void TestMethod1()
        {
            List<OSIAssembly.Token> results = OSIAssembly.Lexer.Lex(System.IO.File.ReadAllText(@"D:\codemastrben\Documents\Projects\Modding\Bionicle\Sample Files\osi stuff\betabase.osa"));
            foreach (OSIAssembly.Token token in results)
            {
                Debug.WriteLine(token.ToString());
            }
        }
    }
}
