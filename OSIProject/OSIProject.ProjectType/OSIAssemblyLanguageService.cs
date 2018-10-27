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
using Microsoft.VisualStudio.Language.Intellisense;
using Microsoft.VisualStudio.Text.Operations;
using Microsoft.VisualStudio.Editor;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.TextManager.Interop;
using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio;
using System.Runtime.InteropServices;

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
                if (t.Type == TokenType.Comment)
                    classification = StandardClassifications.Comment;
                else if (t.Type == TokenType.StringLiteral)
                    classification = StandardClassifications.StringLiteral;
                else if (t.Type == TokenType.Whitespace)
                    classification = StandardClassifications.WhiteSpace;
                else if (t.Type == TokenType.Comma)
                    classification = StandardClassifications.Other;
                else if (t.Type == TokenType.NumberLiteral)
                    classification = StandardClassifications.NumberLiteral;
                else if (t.Type == TokenType.Keyword)
                {
                    if (Language.OSIAssembly.Language.BlueKeywords.Contains(t.Content))
                        classification = StandardClassifications.Keyword;
                    else if (Language.OSIAssembly.Language.BlockKeywords.Contains(t.Content))
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

                for (int r = Regions.Count - 1; r >= 0; r--)
                {
                    if (span.Contains(new Span(Regions[r].StartIndex, Regions[r].Length)))
                    {
                        Regions.RemoveAt(r);
                    }
                }

                int t = 0;
                Stack<FoldRegion> regions = new Stack<FoldRegion>();
                while (t < Tokens.Count)
                {
                    Token token = Tokens[t];
                    if (token.Type == TokenType.Keyword)
                    {
                        if (token.Content == "begin")
                        {
                            FoldRegion newRegion = new FoldRegion(token.StartIndex);
                            if (t < Tokens.Count - 1)
                            {
                                Token blockType = Tokens[t + 1];
                                newRegion.CollapsedText = blockType.Content;
                                if (blockType.Type == TokenType.Keyword && t < Tokens.Count - 2)
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
                        if (lineTokens.Count > 0 && lineTokens[0].Type == TokenType.Keyword && lineTokens[0].Content == "end")
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
                                    if (startLineTokens.Count > 0 && startLineTokens[0].Type == TokenType.Keyword)
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
                                    if (endLineTokens.Count > 0 && endLineTokens[0].Type == TokenType.Keyword)
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
                }

                // Make sure we don't invalidate past the end of the file (can be caused by killing fold regions that were where we would now consider out of bounds)
                if (invalidateRange.End > Snapshot.Length)
                {
                    invalidateRange = new Span(invalidateRange.Start, Snapshot.Length - invalidateRange.Start);
                }

                Parse(invalidateRange);
            }

        }

        [Export(typeof(ICompletionSourceProvider))]
        [ContentType("osiasm")]
        [Name("OSI Assembly Token Completion")]
        internal sealed class CompletionSourceProvider : ICompletionSourceProvider
        {
            [Import]
            internal ITextStructureNavigatorSelectorService NavigatorService { get; set; }

            public ICompletionSource TryCreateCompletionSource(ITextBuffer textBuffer)
            {
                return new CompletionSource(this, textBuffer);
            }
        }

        internal sealed class CompletionSource : ICompletionSource
        {
            private CompletionSourceProvider SourceProvider;
            private ITextBuffer Buffer;

            //private List<Completion> keywordCompletions = new List<Completion>();
            private List<Completion> blockCompletions = new List<Completion>();

            public CompletionSource(CompletionSourceProvider sourceProvider, ITextBuffer buffer)
            {
                SourceProvider = sourceProvider;
                Buffer = buffer;

                /*foreach (string s in Language.OSIAssembly.Language.BlueKeywords)
                {
                    keywordCompletions.Add(new Completion(s, s, s + " Keyword", null, null));
                }
                keywordCompletions.Sort(new Comparison<Completion>((c1, c2) => c1.DisplayText.CompareTo(c2.DisplayText)));*/
                foreach (string s in Language.OSIAssembly.Language.BlockKeywords)
                {
                    blockCompletions.Add(new Completion(s, s, s + " Block", null, null));
                }
                blockCompletions.Sort(new Comparison<Completion>((c1, c2) => c1.DisplayText.CompareTo(c2.DisplayText)));
            }

            private ITrackingSpan FindTokenSpanAtPosition(ITrackingPoint point, ICompletionSession session)
            {
                SnapshotPoint currentPoint = (session.TextView.Caret.Position.BufferPosition) - 1;
                ITextStructureNavigator navigator = SourceProvider.NavigatorService.GetTextStructureNavigator(Buffer);
                TextExtent extent = navigator.GetExtentOfWord(currentPoint);
                SnapshotSpan span;
                if (extent.IsSignificant)
                {
                    span = extent.Span;
                }
                else
                {
                    span = new SnapshotSpan(session.TextView.TextBuffer.CurrentSnapshot, currentPoint + 1, 0);
                }
                return currentPoint.Snapshot.CreateTrackingSpan(span, SpanTrackingMode.EdgeInclusive);
            }

            public void AugmentCompletionSession(ICompletionSession session, IList<CompletionSet> completionSets)
            {
                int triggerPosition = session.GetTriggerPoint(Buffer).GetPosition(Buffer.CurrentSnapshot);
                ITextSnapshotLine line = Buffer.CurrentSnapshot.GetLineFromPosition(triggerPosition);
                List<Token> lineTokens = Lexer.Lex(line.GetText());
                int currentTokenIndex = -1;
                Token previousToken = null; // The rightmost token whose start index is to the left of the trigger position.
                for (int i = 0; i < lineTokens.Count; i++)
                {
                    if (lineTokens[i].StartIndex <= triggerPosition - line.Start.Position && lineTokens[i].StartIndex + lineTokens[i].Length >= triggerPosition - line.Start.Position)
                    {
                        currentTokenIndex = i;
                    }
                    if (lineTokens[i].StartIndex < triggerPosition - line.Start.Position)
                    {
                        previousToken = lineTokens[i];
                    }
                }
                ITrackingSpan span = FindTokenSpanAtPosition(session.GetTriggerPoint(Buffer), session);

                if (lineTokens.Count == 0 || currentTokenIndex == 0) // No tokens yet or working on the first: Get from block context
                {
                    // find the containing 'begin' line
                    List<Token> tokens = null;
                    int nest = 1; // increase for an 'end', increase for a 'begin', and we have found our target when we hit 'begin' when 'nest' becomes 0
                    for (int i = line.LineNumber - 1; i >= 0; i--)
                    {
                        ITextSnapshotLine prevLine = Buffer.CurrentSnapshot.GetLineFromLineNumber(i);
                        tokens = Lexer.Lex(prevLine.GetText());
                        if (tokens.Count > 0)
                        {
                            if (tokens[0].Type == TokenType.Keyword)
                            {
                                if (tokens[0].Content == "end")
                                {
                                    nest++;
                                }
                                else if (tokens[0].Content == "begin")
                                {
                                    nest--;
                                    if (nest == 0)
                                    {
                                        break;
                                    }
                                }
                            }
                        }
                        tokens = null;
                    }

                    string blockType;
                    if (tokens != null && tokens.Count > 1)
                    {
                        blockType = tokens[1].Content;
                    }
                    else if (tokens == null)
                    {
                        blockType = null;
                    }
                    else
                    {
                        return; // No completions for invalid syntax ('begin' without a block type)
                    }

                    BlockContext context = null;
                    context = Language.OSIAssembly.Language.GetBlockContext(blockType);
                    List<Completion> completions = new List<Completion>();
                    foreach (string s in context.ValidFirstTokens)
                    {
                        completions.Add(new Completion(s, s, null, null, null));
                    }

                    completionSets.Add(new CompletionSet("Keywords", "Keywords", span, completions, null));
                }
                else if (lineTokens.Count > 0 && 
                    ((previousToken != null && previousToken.Type == TokenType.Keyword && previousToken.Content == "begin")
                        || previousToken == lineTokens[currentTokenIndex] && currentTokenIndex > 0 && lineTokens[currentTokenIndex - 1].Type == TokenType.Keyword && lineTokens[currentTokenIndex - 1].Content == "begin"))
                {
                    completionSets.Add(new CompletionSet("Blocks", "Blocks", span, blockCompletions, null));
                }
            }

            private bool IsDisposed;
            public void Dispose()
            {
                if (!IsDisposed)
                {
                    GC.SuppressFinalize(this);
                    IsDisposed = true;
                }
            }
        }

        [Export(typeof(IVsTextViewCreationListener))]
        [ContentType("osiasm")]
        [TextViewRole(PredefinedTextViewRoles.Editable)]
        [Name("OSI Assembly Token Completion Handler")]
        internal sealed class CompletionHandlerProvider : IVsTextViewCreationListener
        {
            [Import]
            internal IVsEditorAdaptersFactoryService AdapterService { get; set; }
            [Import]
            internal ICompletionBroker CompletionBroker { get; set; }
            [Import]
            internal SVsServiceProvider ServiceProvider { get; set; }

            public void VsTextViewCreated(IVsTextView textViewAdapter)
            {
                ITextView textView = AdapterService.GetWpfTextView(textViewAdapter);
                if (textView == null)
                    return;

                Func<CompletionCommandHandler> createCommandHandler = delegate () { return new CompletionCommandHandler(textViewAdapter, textView, this); };
                textView.Properties.GetOrCreateSingletonProperty(createCommandHandler);
            }
        }

        internal sealed class CompletionCommandHandler : IOleCommandTarget
        {
            private IOleCommandTarget NextCommandHandler;
            private ITextView TextView;
            private CompletionHandlerProvider Provider;
            private ICompletionSession Session;

            internal CompletionCommandHandler(IVsTextView textViewAdapter, ITextView textView, CompletionHandlerProvider provider)
            {
                this.TextView = textView;
                this.Provider = provider;
                textViewAdapter.AddCommandFilter(this, out NextCommandHandler);
            }

            public int QueryStatus(ref Guid pguidCmdGroup, uint cCmds, OLECMD[] prgCmds, IntPtr pCmdText)
            {
                return NextCommandHandler.QueryStatus(ref pguidCmdGroup, cCmds, prgCmds, pCmdText);
            }

            public int Exec(ref Guid pguidCmdGroup, uint nCmdID, uint nCmdexecopt, IntPtr pvaIn, IntPtr pvaOut)
            {
                if (VsShellUtilities.IsInAutomationFunction(Provider.ServiceProvider))
                {
                    return NextCommandHandler.Exec(ref pguidCmdGroup, nCmdID, nCmdexecopt, pvaIn, pvaOut);
                }
                //make a copy of this so we can look at it after forwarding some commands
                uint commandID = nCmdID;
                char typedChar = char.MinValue;
                //make sure the input is a char before getting it
                if (pguidCmdGroup == VSConstants.VSStd2K && nCmdID == (uint)VSConstants.VSStd2KCmdID.TYPECHAR)
                {
                    typedChar = (char)(ushort)Marshal.GetObjectForNativeVariant(pvaIn);
                }

                //check for a commit character
                if (nCmdID == (uint)VSConstants.VSStd2KCmdID.RETURN
                    || nCmdID == (uint)VSConstants.VSStd2KCmdID.TAB
                    || (char.IsWhiteSpace(typedChar) || char.IsPunctuation(typedChar)))
                {
                    //check for a selection
                    if (Session != null && !Session.IsDismissed)
                    {
                        //if the selection is fully selected, commit the current session
                        if (Session.SelectedCompletionSet.SelectionStatus.IsSelected)
                        {
                            Session.Commit();
                            // also, don't add the character to the buffer on Tab
                            if (nCmdID == (uint)VSConstants.VSStd2KCmdID.TAB)
                            {
                                return VSConstants.S_OK;
                            }
                        }
                        else
                        {
                            //if there is no selection, dismiss the session
                            Session.Dismiss();
                        }
                    }
                }

                //pass along the command so the char is added to the buffer
                int retVal = NextCommandHandler.Exec(ref pguidCmdGroup, nCmdID, nCmdexecopt, pvaIn, pvaOut);
                bool handled = false;
                if (!typedChar.Equals(char.MinValue) && char.IsLetterOrDigit(typedChar))
                {
                    if (Session == null || Session.IsDismissed) // If there is no active session, bring up completion
                    {
                        this.TriggerCompletion();
                        if (Session != null) // Session could be null if the completion has no suggestions.
                            Session.Filter();
                    }
                    else    //the completion session is already active, so just filter
                    {
                        string typedText = Session.CompletionSets[0].ApplicableTo.GetText(Session.TextView.TextSnapshot);
                        Session.Filter();
                    }
                    handled = true;
                }
                else if (Session == null && typedChar == ' ') // show help after 'begin' (and TODO: after instructions)
                {
                    int triggerPosition = TextView.Caret.Position.BufferPosition.Position;
                    ITextSnapshotLine line = TextView.TextBuffer.CurrentSnapshot.GetLineFromPosition(triggerPosition);
                    List<Token> lineTokens = Lexer.Lex(line.GetText());
                    Token lastTokenBeforeCaret = null;
                    foreach (Token t in lineTokens)
                    {
                        if (t.StartIndex + t.Length <= triggerPosition - line.Start.Position)
                            lastTokenBeforeCaret = t;
                        else
                            break;
                    }

                    if (lastTokenBeforeCaret != null && lastTokenBeforeCaret.Type == TokenType.Keyword && lastTokenBeforeCaret.Content == "begin")
                        TriggerCompletion();

                }
                else if (commandID == (uint)VSConstants.VSStd2KCmdID.BACKSPACE   //redo the filter if there is a deletion
                    || commandID == (uint)VSConstants.VSStd2KCmdID.DELETE)
                {
                    if (Session != null && !Session.IsDismissed)
                        Session.Filter();
                    handled = true;
                }
                if (handled) return VSConstants.S_OK;
                return retVal;
            }

            private bool TriggerCompletion()
            {
                //the caret must be in a non-projection location 
                SnapshotPoint? caretPoint =
                TextView.Caret.Position.Point.GetPoint(
                textBuffer => (!textBuffer.ContentType.IsOfType("projection")), PositionAffinity.Predecessor);
                if (!caretPoint.HasValue)
                {
                    return false;
                }

                Session = Provider.CompletionBroker.CreateCompletionSession(TextView,
                    caretPoint.Value.Snapshot.CreateTrackingPoint(caretPoint.Value.Position, PointTrackingMode.Positive),
                    true);

                //subscribe to the Dismissed event on the session 
                Session.Dismissed += Session_Dismissed;
                Session.Start();

                return true;
            }

            private void Session_Dismissed(object sender, EventArgs e)
            {
                Session.Dismissed -= this.Session_Dismissed;
                Session = null;
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
