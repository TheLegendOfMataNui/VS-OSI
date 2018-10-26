using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.ComponentModel.Composition;
using Microsoft.VisualStudio.Text.Classification;
using Microsoft.VisualStudio.Utilities;
using Microsoft.VisualStudio.Text.Tagging;
using Microsoft.VisualStudio.Text.Outlining;
using Microsoft.VisualStudio.Text;
using OSIProject.Language.OSIAssembly;
using Microsoft.VisualStudio.Language.StandardClassification;

namespace OSIProject
{

    internal static class OSIAssemblyLanguageService
    {
        [Export]
        [Name("osiasm")]
        [BaseDefinition("text")]
        internal static ContentTypeDefinition OSIAsmContentType;

        [Export]
        [FileExtension(".osa")]
        [ContentType("osiasm")]
        internal static FileExtensionToContentTypeDefinition OSIAsmExtensionToContentType;

        [Export(typeof(ITaggerProvider))]
        [TagType(typeof(IClassificationTag))]
        [ContentType("osiasm")]
        internal sealed class ColoringTaggerProvider : ITaggerProvider
        {
            [Import]
            internal IClassificationTypeRegistryService ClassificationRegistry { get; set; }

            [Import]
            internal IStandardClassificationService StandardClassifications { get; set; }

            public ITagger<T> CreateTagger<T>(ITextBuffer buffer) where T : ITag
            {
                Func<ITagger<T>> sc = delegate () { return new ColoringTagger(buffer, ClassificationRegistry, StandardClassifications) as ITagger<T>; };
                return buffer.Properties.GetOrCreateSingletonProperty(sc);
            }
        }

        internal sealed class ColoringTagger : ITagger<IClassificationTag>
        {
            private static readonly string[] BlueKeywords = new string[] { "begin", "end" };
            private static readonly string[] BlockKeywords = new string[] { "metadata", "strings", "globals", "symbols", "sources", "functions", "classes", "class", "subroutine" };

            private ITextBuffer Buffer { get; }
            private ITextSnapshot Snapshot { get; set; }
            private IClassificationTypeRegistryService ClassificationRegistry { get; }
            private IStandardClassificationService StandardClassifications { get; }

            //private Dictionary<int, List<Token>> Lines = new Dictionary<int, List<Token>>(); // The token start indexes are relative to the beginning of that line!!!

            public event EventHandler<SnapshotSpanEventArgs> TagsChanged;

            public ColoringTagger(ITextBuffer buffer, IClassificationTypeRegistryService classificationRegistry, IStandardClassificationService standardClassifications)
            {
                this.Buffer = buffer;
                this.Snapshot = this.Buffer.CurrentSnapshot;
                this.ClassificationRegistry = classificationRegistry;
                this.StandardClassifications = standardClassifications;
                this.Buffer.Changed += Buffer_Changed;
                //Parse(0, Snapshot.LineCount - 1);
            }

            private void Buffer_Changed(object sender, TextContentChangedEventArgs e)
            {
                Span s = e.Changes[0].NewSpan;
                foreach (ITextChange change in e.Changes)
                {
                    s = s.Union(change.NewSpan);
                }
                Snapshot = e.After;

                TagsChanged?.Invoke(this, new SnapshotSpanEventArgs(new SnapshotSpan(Snapshot, s)));
            }

            /*private void Parse(int startLine, int endLine)
            {
                for (int l = startLine; l <= endLine; l++)
                {
                    // Clear or add the given lines
                    if (Lines.ContainsKey(l))
                        Lines.Remove(l);

                    // Parse the given lines and store the result
                    ITextSnapshotLine line = Snapshot.GetLineFromLineNumber(l);
                    string lineText = line.GetText();
                    List<Token> tokens = Lexer.Lex(lineText, true);
                    Lines.Add(l, tokens);
                }


                int start = Snapshot.GetLineFromLineNumber(startLine).Start.Position;
                int end = Snapshot.GetLineFromLineNumber(endLine).End.Position;
                TagsChanged?.Invoke(this, new SnapshotSpanEventArgs(new SnapshotSpan(Snapshot, new Span(start, end - start))));
            }*/

            private IClassificationTag ClassifyToken(Token t)
            {
                IClassificationType classification = this.StandardClassifications.ExcludedCode;
                if (t.Type == Token.TokenType.Comment)
                    classification = StandardClassifications.Comment;
                else if (t.Type == Token.TokenType.StringLiteral)
                    classification = StandardClassifications.StringLiteral;
                else if (t.Type == Token.TokenType.Whitespace)
                    classification = StandardClassifications.WhiteSpace;
                else if (t.Type == Token.TokenType.Comma)
                    classification = StandardClassifications.Other;
                else if (t.Type == Token.TokenType.NumberLiteral)
                    classification = StandardClassifications.NumberLiteral;
                else if (t.Type == Token.TokenType.Keyword)
                {
                    if (BlueKeywords.Contains(t.Content))
                        classification = StandardClassifications.Keyword;
                    else if (BlockKeywords.Contains(t.Content))
                        classification = StandardClassifications.Keyword; // no symbolreference
                    else
                        classification = StandardClassifications.Identifier;
                }

                return new ClassificationTag(classification);
            }

            public IEnumerable<ITagSpan<IClassificationTag>> GetTags(NormalizedSnapshotSpanCollection spans)
            {
                /*foreach (SnapshotSpan span in spans)
                {
                    for (int line = span.Snapshot.GetLineNumberFromPosition(span.Start.Position); line <= span.Snapshot.GetLineNumberFromPosition(span.End.Position); line++) 
                    {
                        if (!Lines.ContainsKey(line))
                        {
                            Parse(line, line);
                        }

                        foreach (Token t in Lines[line])
                        {
                            yield return new TagSpan<IClassificationTag>(new SnapshotSpan(span.Snapshot, t.StartIndex + span.Snapshot.GetLineFromLineNumber(line).Start.Position, t.Length), ClassifyToken(t));
                        }
                    }
                }*/

                foreach (SnapshotSpan span in spans)
                {
                    for (int line = span.Snapshot.GetLineNumberFromPosition(span.Start.Position); line <= span.Snapshot.GetLineNumberFromPosition(span.End.Position); line++)
                    {
                        ITextSnapshotLine l = span.Snapshot.GetLineFromLineNumber(line);
                        List<Token> tokens = Lexer.Lex(l.GetText(), true);
                        foreach (Token t in tokens)
                        {
                            yield return new TagSpan<IClassificationTag>(new SnapshotSpan(span.Snapshot, l.Start.Position + t.StartIndex, t.Length), ClassifyToken(t));
                        }
                    }
                }
            }
        }

        [Export(typeof(ITaggerProvider))]
        [TagType(typeof(IOutliningRegionTag))]
        [ContentType("osiasm")]
        internal sealed class OutliningTaggerProvider : ITaggerProvider
        {
            public ITagger<T> CreateTagger<T>(ITextBuffer buffer) where T : ITag
            {
                Func<ITagger<T>> sc = delegate () { return new OutliningTagger(buffer) as ITagger<T>; };
                return buffer.Properties.GetOrCreateSingletonProperty(sc);
            }
        }

        internal sealed class OutliningTagger : ITagger<IOutliningRegionTag>
        {
            private class FoldRegion
            {
                public int StartIndex;
                public int Length;
                public string CollapsedText;
                public string TooltipText;
                public int ClosingTokenLength;

                public FoldRegion(int startIndex)
                {
                    this.StartIndex = startIndex;
                }
            }

            private ITextBuffer Buffer;
            private ITextSnapshot Snapshot;

            private List<FoldRegion> Regions = new List<FoldRegion>();

            public event EventHandler<SnapshotSpanEventArgs> TagsChanged;

            public OutliningTagger(ITextBuffer buffer)
            {
                Buffer = buffer;
                Snapshot = Buffer.CurrentSnapshot;
                this.Buffer.Changed += Buffer_Changed;
                Parse(new Span(0, Snapshot.Length));
            }

            public IEnumerable<ITagSpan<IOutliningRegionTag>> GetTags(NormalizedSnapshotSpanCollection spans)
            {
                foreach (FoldRegion region in Regions)
                    yield return new TagSpan<IOutliningRegionTag>(new SnapshotSpan(Snapshot, region.StartIndex, region.Length), new OutliningRegionTag(true, true, region.CollapsedText, region.TooltipText));
            }

            private void Parse(Span span)
            {
                List<Token> Tokens = null;
                Tokens = Lexer.Lex(Snapshot.GetText(span));
                //Spans.Clear();

                for (int r = Regions.Count - 1; r >= 0; r--)
                {
                    if (span.Contains(new Span(Regions[r].StartIndex, Regions[r].Length)))
                    {
                        Regions.RemoveAt(r);
                    }
                }

                /*int t = 0;
                int startIndex = 0;
                while (t < Tokens.Count)
                {
                    if (Tokens[t].Type == Token.TokenType.Keyword && Tokens[t].Content == "begin")
                    {
                        startIndex = Tokens[t].StartIndex;
                        while (Tokens[t].Type != Token.TokenType.Keyword || Tokens[t].Content != "end")
                        {
                            t++;
                        }
                        int length = Tokens[t].StartIndex + Tokens[t].Length - startIndex;
                        Spans.Add(new Tuple<int, int>(startIndex, length));
                    }
                    t++;
                }*/

                int t = 0;
                Stack<FoldRegion> regions = new Stack<FoldRegion>();
                while (t < Tokens.Count)
                {
                    Token token = Tokens[t];
                    if (token.Type == Token.TokenType.Keyword)
                    {
                        if (token.Content == "begin")
                        {
                            FoldRegion newRegion = new FoldRegion(token.StartIndex);
                            if (t < Tokens.Count - 1)
                            {
                                Token blockType = Tokens[t + 1];
                                newRegion.CollapsedText = blockType.Content;
                                if (blockType.Type == Token.TokenType.Keyword && t < Tokens.Count - 2)
                                {
                                    if (blockType.Content == "class")
                                    {
                                        Token className = Tokens[t + 2];
                                        newRegion.CollapsedText += " " + className.Content;
                                    }
                                    else if (blockType.Content == "subroutine")
                                    {
                                        Token subName = Tokens[t + 2];
                                        newRegion.CollapsedText += " " + subName.Content;
                                    }
                                }
                            }
                            newRegion.CollapsedText += "...";
                            regions.Push(newRegion);
                        }
                        else if (token.Content == "end")
                        {
                            if (regions.Count > 0)
                            {
                                FoldRegion region = regions.Pop();
                                region.Length = token.StartIndex + token.Length - region.StartIndex;
                                region.ClosingTokenLength = token.Content.Length;

                                // Adjust for the given span
                                region.StartIndex += span.Start;
                                Regions.Add(region);
                            }
                        }
                    }
                    t++;
                }

                this.TagsChanged?.Invoke(this, new SnapshotSpanEventArgs(new SnapshotSpan(Snapshot, span)));
            }

            // Removes any
            /*private void InvalidateSpan(Span span)
            {

            }*/

            private void Buffer_Changed(object sender, TextContentChangedEventArgs e)
            {
                if (e.Changes.Count == 0)
                    return;
                //throw new NotImplementedException();
                Snapshot = e.After;

                // Update the already changed regions
                Span invalidateRange = e.Changes[0].NewSpan; // All fold regions contained here will be deleted and reparsed
                foreach (ITextChange change in e.Changes)
                {
                    // expand invalidateRange to include change.NewSpan
                    /*if (change.NewPosition < invalidateRange.Start)
                        invalidateRange = new Span(change.NewPosition, invalidateRange.End - change.NewPosition);
                    if (change.NewEnd > invalidateRange.End)
                        invalidateRange = new Span(invalidateRange.Start, change.NewEnd - invalidateRange.Start);*/
                    invalidateRange = invalidateRange.Union(change.NewSpan);

                    // - Fix up starts and lengths of spans before/after/around
                    // - Invalidate spans intersecting
                    for (int r = Regions.Count - 1; r >= 0; r--)
                    {
                        FoldRegion region = Regions[r];
                        if (region.StartIndex < change.OldPosition)
                        {
                            // Starts before the change - check end position
                            if (region.StartIndex + region.Length < change.OldPosition)
                            {
                                // Ends before the change - do nothing
                            }
                            else if (region.StartIndex + region.Length - region.ClosingTokenLength > change.OldEnd)
                            {
                                // Ends after the change - adjust length
                                region.Length += change.Delta;
                            }
                            else
                            {
                                // Ends during the change - delete and invalidate
                                Regions.RemoveAt(r);
                                invalidateRange = invalidateRange.Union(new Span(region.StartIndex, region.Length));
                            }
                        }
                        else if (region.StartIndex >= change.OldEnd)
                        {
                            // Starts after the change - adjust start
                            region.StartIndex += change.Delta;
                        }
                        else
                        {
                            // Starts in the change - delete and invalidate
                            Regions.RemoveAt(r);
                            invalidateRange = invalidateRange.Union(new Span(region.StartIndex, region.Length));
                        }
                    }

                    // Check to see if we just added a tail part to a now-valid folding region, and if we did, seek upwards until we find the corresponding top.
                    for (int line = Snapshot.GetLineNumberFromPosition(change.NewPosition); line <= Snapshot.GetLineNumberFromPosition(change.NewEnd); line++)
                    {
                        string lineText = Snapshot.GetLineFromLineNumber(line).GetText();
                        List<Token> lineTokens = Lexer.Lex(lineText);
                        if (lineTokens.Count > 0 && lineTokens[0].Type == Token.TokenType.Keyword && lineTokens[0].Content == "end")
                        {
                            if (lineTokens[0].Content == "end")
                            {
                                // Scan upwards until we find a matching begin, or the beginning of the file.
                                int startLine = line - 1;
                                int endCount = 1; // increase by one for every end that we pass, and decrease for every begin.
                                while (startLine >= 0 && endCount > 0)
                                {
                                    string startLineText = Snapshot.GetLineFromLineNumber(startLine).GetText();
                                    List<Token> startLineTokens = Lexer.Lex(startLineText);
                                    if (startLineTokens.Count > 0 && startLineTokens[0].Type == Token.TokenType.Keyword)
                                    {
                                        if (startLineTokens[0].Content == "end")
                                        {
                                            endCount++;
                                        }
                                        else if (startLineTokens[0].Content == "begin")
                                        {
                                            endCount--;
                                        }
                                    }
                                    startLine--;
                                }
                                startLine++;
                                int start = Snapshot.GetLineFromLineNumber(startLine).Start.Position;
                                int end = Snapshot.GetLineFromLineNumber(line).End.Position;

                                invalidateRange = invalidateRange.Union(new Span(start, end - start));
                            }
                            else if (lineTokens[0].Content == "begin")
                            {
                                // Scan downwards until we find a matching end, or the end of the file.
                                int endLine = line + 1;
                                int beginCount = 1;
                                while (endLine < Snapshot.LineCount && beginCount > 0)
                                {
                                    string endLineText = Snapshot.GetLineFromLineNumber(endLine).GetText();
                                    List<Token> endLineTokens = Lexer.Lex(endLineText);
                                    if (endLineTokens.Count > 0 && endLineTokens[0].Type == Token.TokenType.Keyword)
                                    {
                                        if (endLineTokens[0].Content == "begin")
                                        {
                                            beginCount++;
                                        }
                                        else if (endLineTokens[0].Content == "end")
                                        {
                                            beginCount--;
                                        }
                                    }
                                    endLine++;
                                }
                                endLine--;
                                int start = Snapshot.GetLineFromLineNumber(line).Start.Position;
                                int end = Snapshot.GetLineFromLineNumber(endLine).End.Position;

                                invalidateRange = invalidateRange.Union(new Span(start, end - start));
                            }
                        }
                    }
                    //InvalidateChange(change.OldSpan, change.NewSpan);
                    //Parse(change.NewSpan);
                }

                Parse(invalidateRange);
            }

        }

        /// <summary>
        /// Returns a Span that completely contains both span1 and span2.
        /// </summary>
        /// <param name="span1"></param>
        /// <param name="span2"></param>
        /// <returns></returns>
        private static Span Union(this Span span1, Span span2)
        {
            Span result = span1;
            if (span2.Start < result.Start)
                result = new Span(span2.Start, result.End - span2.Start);
            if (span2.End > result.End)
                result = new Span(result.Start, span2.End - result.Start);
            return result;
        }
    }

}
