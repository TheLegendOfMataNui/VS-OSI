using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OSIProject.Language.OSIAssembly
{
    public static class Language
    {
        public static readonly HintInfo BeginHint = new HintInfo("begin", "keyword", "Begins a block of the given type.\nExample: 'begin class'");
        public static readonly HintInfo EndHint = new HintInfo("end", "keyword", "Ends a block that was begun with the 'begin' keyword.");
        // Blocks
        public static readonly HintInfo MetadataHint = new HintInfo("metadata", "blocktype", "Begins a block that has information about the file.");
        public static readonly HintInfo StringsHint = new HintInfo("strings", "blocktype", "Begins a block that lists all the strings used in the file. They can be referred to later by their index in this block.");
        public static readonly HintInfo GlobalsHint = new HintInfo("globals", "blocktype", "Begins a block that lists all the global variables used in the file. They can be referred to later by their index in this block.");
        public static readonly HintInfo SymbolsHint = new HintInfo("symbols", "blocktype", "Begins a block that lists all the symbols (names) used in the file. They can be referred to later by their index in this block.");
        public static readonly HintInfo SourcesHint = new HintInfo("sources", "blocktype", "Begins a block that lists all the source filesnames used in the file. They can be referred to later by their index in this block.");
        public static readonly HintInfo FunctionsHint = new HintInfo("functions", "blocktype", "Begins a block that describes all the functions that this OSI defines.");
        public static readonly HintInfo ClassesHint = new HintInfo("classes", "blocktype", "Begins a block that defines all the classes that this OSI defines.");
        public static readonly HintInfo ClassHint = new HintInfo("class", "class", "Begins a block that defines the members (properties and methods) of a class.");
        public static readonly HintInfo SubroutineHint = new HintInfo("subroutine", "blocktype", "Begins a block that defines the behavior of a method or function.");
        // Statements
        public static readonly HintInfo VersionHint = new HintInfo("version", "version", "Sets the OSI version of the file. \nExample: \version 4, 1'");
        public static readonly HintInfo StringHint = new HintInfo("string", "string", "Defines text that can be referenced by its index. \nExample: 'string \"text\"'");
        public static readonly HintInfo GlobalHint = new HintInfo("global", "global", "Declares a global variable that can be referenced to be its index. \nExample: 'global \"globalclass\"'");
        public static readonly HintInfo SymbolHint = new HintInfo("symbol", "symbol", "Declares a name that can be referenced by its index by objects in this file. \nExample: 'symbol \"name\"'");
        public static readonly HintInfo SourceHint = new HintInfo("source", "source", "Declares a filename that can be referenced in the file as a source code filename. \nExample: 'source \"C:\\file.osa\"'");
        public static readonly HintInfo FunctionHint = new HintInfo("function", "function", "Declares a function that will be implemented in this file, by stating its name, the parameter count, and the index of the subroutine that implements it. \nExample: `function \"MyTestFunction\", 3, 1'");
        // Members
        public static readonly HintInfo PropertyHint = new HintInfo("property", "property", "Declares an instance variable for the containing class. \nExample: 'property \"thing\"'");
        public static readonly HintInfo MethodHint = new HintInfo("method", "function", "Declares an instance method for the containing class. \nExample: 'method \"thing\", 1'");

        public static readonly HintInfo[] BlueKeywords = new HintInfo[] { BeginHint, EndHint };
        public static readonly HintInfo[] BlockKeywords = new HintInfo[] { MetadataHint, StringsHint, GlobalsHint, SymbolsHint, SourcesHint, FunctionsHint, ClassesHint, ClassHint, SubroutineHint };
        public static readonly HintInfo[] TopLevelBlockKeywords = new HintInfo[] { MetadataHint, StringsHint, GlobalsHint, SymbolsHint, SourcesHint, FunctionsHint, ClassesHint, SubroutineHint };
        public static readonly HintInfo[] MetadataKeywords = new HintInfo[] { VersionHint };
        public static readonly HintInfo[] StringsKeywords = new HintInfo[] { StringHint };
        public static readonly HintInfo[] GlobalsKeywords = new HintInfo[] { GlobalHint };
        public static readonly HintInfo[] SymbolsKeywords = new HintInfo[] { SymbolHint };
        public static readonly HintInfo[] SourcesKeywords = new HintInfo[] { SourceHint };
        public static readonly HintInfo[] FunctionsKeywords = new HintInfo[] { FunctionHint };
        public static readonly HintInfo[] ClassesKeywords = new HintInfo[] { BeginHint, EndHint };
        public static readonly HintInfo[] ClassKeywords = new HintInfo[] { PropertyHint, MethodHint };
        public static readonly HintInfo[] SubroutineKeywords = new HintInfo[]
        {
            new HintInfo("JumpTarget", "instruction", ""),
            new HintInfo("PushConstanti32JumpTarget", "instruction", ""),
            new HintInfo("BranchTarget", "instruction", ""),
            new HintInfo("BranchAlwaysTarget", "instruction", ""),
            new HintInfo("CompareAndBranchIfFalseTarget", "instruction", ""),
            new HintInfo("PushConstantStringString", "instruction", ""),
            new HintInfo("GetThisMemberFunctionString", "instruction", ""),
            new HintInfo("GetThisMemberValueString", "instruction", ""),
            new HintInfo("SetThisMemberValueString", "instruction", ""),
            new HintInfo("GetMemberFunctionString", "instruction", ""),
            new HintInfo("GetMemberValueString", "instruction", ""),
            new HintInfo("SetMemberValueString", "instruction", ""),
            new HintInfo("GetGameVariableString", "instruction", ""),
            new HintInfo("SetGameVariableString", "instruction", ""),
            new HintInfo("CallGameFunctionString", "instruction", ""),
            new HintInfo("CallGameFunctionFromStringString", "instruction", ""),
            new HintInfo("Nop", "instruction", ""),
            new HintInfo("DebugOn", "instruction", ""),
            new HintInfo("DebugOff", "instruction", ""),
            new HintInfo("LineNumber", "instruction", ""),
            new HintInfo("LineNumberAlt1", "instruction", ""),
            new HintInfo("LineNumberAlt2", "instruction", ""),
            new HintInfo("SetMemberValue", "instruction", ""),
            new HintInfo("GetMemberValue", "instruction", ""),
            new HintInfo("GetMemberFunction", "instruction", ""),
            new HintInfo("CreateObject", "instruction", ""),
            new HintInfo("MemberFunctionArgumentCheck", "instruction", ""),
            new HintInfo("SetThisMemberValue", "instruction", ""),
            new HintInfo("GetThisMemberValue", "instruction", ""),
            new HintInfo("GetThisMemberFunction", "instruction", ""),
            new HintInfo("GetMemberValueFromString", "instruction", ""),
            new HintInfo("GetMemberFunctionFromString", "instruction", ""),
            new HintInfo("SetMemberValueFromString", "instruction", ""),
            new HintInfo("GetVariableValue", "instruction", ""),
            new HintInfo("SetVariableValue", "instruction", ""),
            new HintInfo("CreateStackVariables", "instruction", ""),
            new HintInfo("IncrementVariable", "instruction", ""),
            new HintInfo("DecrementVariable", "instruction", ""),
            new HintInfo("Pop", "instruction", ""),
            new HintInfo("PopN", "instruction", ""),
            new HintInfo("Swap", "instruction", ""),
            new HintInfo("Pull", "instruction", ""),
            new HintInfo("DupN", "instruction", ""),
            new HintInfo("Dup", "instruction", ""),
            new HintInfo("PushConstanti32", "instruction", ""),
            new HintInfo("PushConstanti24", "instruction", ""),
            new HintInfo("PushConstanti16", "instruction", ""),
            new HintInfo("PushConstanti8", "instruction", ""),
            new HintInfo("PushConstantf32", "instruction", ""),
            new HintInfo("PushConstant0", "instruction", ""),
            new HintInfo("PushConstantString", "instruction", ""),
            new HintInfo("PushNothing", "instruction", ""),
            new HintInfo("PushConstantColor8888", "instruction", ""),
            new HintInfo("PushConstantColor5551", "instruction", ""),
            new HintInfo("JumpRelative", "instruction", ""),
            new HintInfo("JumpAbsolute", "instruction", ""),
            new HintInfo("Return", "instruction", ""),
            new HintInfo("CompareAndBranchIfFalse", "instruction", ""),
            new HintInfo("BranchAlways", "instruction", ""),
            new HintInfo("EqualTo", "instruction", ""),
            new HintInfo("LessThan", "instruction", ""),
            new HintInfo("GreaterThan", "instruction", ""),
            new HintInfo("LessOrEqual", "instruction", ""),
            new HintInfo("GreaterOrEqual", "instruction", ""),
            new HintInfo("And", "instruction", ""),
            new HintInfo("Or", "instruction", ""),
            new HintInfo("Not", "instruction", ""),
            new HintInfo("BitwiseAnd", "instruction", ""),
            new HintInfo("BitwiseOr", "instruction", ""),
            new HintInfo("BitwiseXor", "instruction", ""),
            new HintInfo("Add", "instruction", ""),
            new HintInfo("Subtract", "instruction", ""),
            new HintInfo("Multiply", "instruction", ""),
            new HintInfo("Divide", "instruction", ""),
            new HintInfo("Power", "instruction", ""),
            new HintInfo("Modulus", "instruction", ""),
            new HintInfo("BitwiseNot", "instruction", ""),
            new HintInfo("ShiftLeft", "instruction", ""),
            new HintInfo("ShiftRight", "instruction", ""),
            new HintInfo("Increment", "instruction", ""),
            new HintInfo("Decrement", "instruction", ""),
            new HintInfo("GetGameVariable", "instruction", ""),
            new HintInfo("SetGameVariable", "instruction", ""),
            new HintInfo("CallGameFunction", "instruction", ""),
            new HintInfo("CallGameFunctionFromString", "instruction", ""),
            new HintInfo("CallGameFunctionDirect", "instruction", ""),
            new HintInfo("CreateArray", "instruction", ""),
            new HintInfo("GetArrayValue", "instruction", ""),
            new HintInfo("ElementsInArray", "instruction", ""),
            new HintInfo("SetArrayValue", "instruction", ""),
            new HintInfo("AppendToArray", "instruction", ""),
            new HintInfo("RemoveFromArray", "instruction", ""),
            new HintInfo("InsertIntoArray", "instruction", ""),
            new HintInfo("SetRedValue", "instruction", ""),
            new HintInfo("SetGreenValue", "instruction", ""),
            new HintInfo("SetBlueValue", "instruction", ""),
            new HintInfo("SetAlphaValue", "instruction", ""),
            new HintInfo("GetRedValue", "instruction", ""),
            new HintInfo("GetGreenValue", "instruction", ""),
            new HintInfo("GetBlueValue", "instruction", ""),
            new HintInfo("GetAlphaValue", "instruction", ""),
            new HintInfo("ConvertToString", "instruction", ""),
            new HintInfo("ConvertToFloat", "instruction", ""),
            new HintInfo("ConvertToInteger", "instruction", ""),
            new HintInfo("IsInteger", "instruction", ""),
            new HintInfo("IsFloat", "instruction", ""),
            new HintInfo("IsString", "instruction", ""),
            new HintInfo("IsAnObject", "instruction", ""),
            new HintInfo("IsGameObject", "instruction", ""),
            new HintInfo("IsArray", "instruction", ""),
            new HintInfo("GetObjectClassID", "instruction", ""),
            new HintInfo("Halt", "instruction", ""),

        };

        private static Dictionary<string, BlockContext> BlockContexts = null;
        private static BlockContext TopContext = null;
        public static BlockContext GetBlockContext(string keyword)
        {
            if (BlockContexts == null)
            {
                BlockContexts = new Dictionary<string, BlockContext>();
                TopContext = new BlockContext(null, BlueKeywords, TopLevelBlockKeywords);
                BlockContexts.Add("metadata", new BlockContext("metadata", MetadataKeywords));
                BlockContexts.Add("strings", new BlockContext("strings", StringsKeywords));
                BlockContexts.Add("globals", new BlockContext("globals", GlobalsKeywords));
                BlockContexts.Add("symbols", new BlockContext("symbols", SymbolsKeywords));
                BlockContexts.Add("sources", new BlockContext("sources", SourcesKeywords));
                BlockContexts.Add("functions", new BlockContext("functions", FunctionsKeywords));
                BlockContexts.Add("classes", new BlockContext("classes", ClassesKeywords, new HintInfo[] { ClassHint }));
                BlockContexts.Add("class", new BlockContext("class", ClassKeywords));
                BlockContexts.Add("subroutine", new BlockContext("subroutine", SubroutineKeywords));
            }
            if (keyword == null)
                return TopContext;
            else if (BlockContexts.ContainsKey(keyword))
                return BlockContexts[keyword];
            else
                return null;
        }
    }
    
    /// <summary>
    /// Defines the code hints that apply to the inside of a particular block type.
    /// </summary>
    public class BlockContext
    {
        public string Keyword { get; }
        public IEnumerable<HintInfo> ValidFirstTokens { get; }
        public IEnumerable<HintInfo> ValidSubBlocks { get; }

        public BlockContext(string keyword, IEnumerable<HintInfo> validFirstTokens, IEnumerable<HintInfo> validSubBlocks = null)
        {
            this.Keyword = keyword;
            this.ValidFirstTokens = validFirstTokens;
            this.ValidSubBlocks = validSubBlocks;
            if (this.ValidSubBlocks == null)
                this.ValidSubBlocks = new List<HintInfo>();
        }
    }

    public class HintInfo
    {
        public string Name { get; }
        public string IconID { get; }
        public string Description { get; }

        public HintInfo(string name, string iconID = "", string description = "")
        {
            this.Name = name;
            this.IconID = iconID;
            this.Description = description;
        }
    }

    public enum TokenType
    {
        Invalid,
        Whitespace,
        Keyword, // begin, end, metadata, strings, sources, functions, classes, property, method, class
        Comment, // ; stuff
        Comma, // ,
        NumberLiteral, // -1049
        StringLiteral, // "thing"
    }

    public class Token
    {
        public string Content { get; }
        public TokenType Type { get; }
        public int StartIndex { get; }
        public int Length { get; }

        public Token(string content, TokenType type, int startIndex, int length)
        {
            this.Content = content;
            this.Type = type;
            this.StartIndex = startIndex;
            this.Length = length;
        }

        public override string ToString()
        {
            return Type.ToString() + ": '" + Content + "'";
        }
    }

    public static class Lexer
    {
        public static List<Token> Lex(string input, bool includeWhitespace = false)
        {
            // This is greatly simplified by the fact that the token type can be uniquely identified by the first character of the token.
            List<Token> results = new List<Token>();
            //string buffer = "";
            TokenType type;
            int startIndex = 0;
            int index = 0;
            bool wasEscapeSlash = false;
            while (index < input.Length)
            {
                //buffer = "";
                startIndex = index;
                char ch = input[index];
                type = ClassifyFirstCharacter(ch);
                while (index < input.Length && CanBePartOfToken(ch, type, index - startIndex))
                {
                    //buffer += ch;
                    index++;
                    if (type == TokenType.StringLiteral && !wasEscapeSlash && ch == '"' && index > startIndex + 1)
                        break;
                    wasEscapeSlash = type == TokenType.StringLiteral && ch == '\\';
                    if (index < input.Length)
                        ch = input[index];
                }

                // Tweak the Infinity and NaN Keywords to NumberLiterals
                string content = input.Substring(startIndex, index - startIndex);
                if (type == TokenType.Keyword && (content == "Infinity" || content == "NaN"))
                    type = TokenType.NumberLiteral;

                if (type != TokenType.Whitespace || includeWhitespace)
                    results.Add(new Token(content, type, startIndex, index - startIndex));
            }

            return results;
        }

        private static TokenType ClassifyFirstCharacter(char character)
        {
            if (character == '"')
                return TokenType.StringLiteral;
            else if (Char.IsLetter(character))
                return TokenType.Keyword;
            else if (Char.IsDigit(character) || character == '-' || character == '+' || character == '.')
                return TokenType.NumberLiteral;
            else if (character == ',')
                return TokenType.Comma;
            else if (character == ';')
                return TokenType.Comment;
            else if (Char.IsWhiteSpace(character))
                return TokenType.Whitespace;
            else
                return TokenType.Invalid;
        }

        private static bool CanBePartOfToken(char character, TokenType type, int position)
        {
            if (type == TokenType.Comment)
                return character != '\n'; // Everything till a line break
            else if (type == TokenType.Comma)
                return character == ',' && position == 0; // Only one character, a comma
            else if (type == TokenType.NumberLiteral)
                return Char.IsNumber(character) || Char.IsLetter(character) || character == '-' || character == '+' || character == '.' || character == 'e' || character == 'E' || character == 'x' || character == 'X' || character == 'o' || character == 'O' || character == 'b' || character == 'B';
            else if (type == TokenType.Invalid)
                return !Char.IsLetterOrDigit(character)
                    && !Char.IsWhiteSpace(character)
                    && character != ','
                    && character != ';'
                    && character != '-'
                    && character != '"';
            else if (type == TokenType.Keyword)
                return Char.IsLetter(character) || Char.IsNumber(character);
            else if (type == TokenType.StringLiteral)
                return character != '\n'; // Anything besides a newline
            else if (type == TokenType.Whitespace)
                return Char.IsWhiteSpace(character);
            else
                throw new ArgumentException("Invalid TokenType in '" + nameof(type) + "'!");
        }
    }
}
